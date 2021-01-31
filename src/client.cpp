#include "client.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace client {

/* game_client_context */

game_client_context::game_client_context(asio::io_context& io, asio::ip::udp::endpoint server_addr)
    : context(io), server_addr(server_addr), connection() {}

game_client_context::~game_client_context() {
    stop();
}

void game_client_context::start() {
    context.connect({asio::ip::udp::v6(), 0}, server_addr);
}

void game_client_context::stop() {
    context.stop();
}

/* lobby_client */

lobby_client::lobby_client(std::shared_ptr<game_client_context> context)
    : context(context), current_state() {}

auto lobby_client::get_state() -> const lobby_state& {
    return current_state;
}

auto lobby_client::get_context() -> const std::shared_ptr<game_client_context>& {
    return context;
}

auto lobby_client::tick([[maybe_unused]] float delta) -> std::optional<message::game_started> {
    context->context.poll_events(*this);

    return current_state.game_start;
}

void lobby_client::start_game() {
    send_message<channel::actions>(context->connection, message::lobby_start_game{});
}

void lobby_client::on_connect(const connection_ptr& conn) {
    std::cout << "Connected." << std::endl;
    context->connection = conn;
}

void lobby_client::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected. Reason: " << ec.category().name() << ": " << ec.message() << std::endl;
    current_state.me = std::nullopt;
    context->connection = nullptr;
}

void lobby_client::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) {
    auto msg = receive_message(data);

    std::visit(ember::utility::overload {
        [&](const message::lobby_state_update& m) {
            current_state.me = m.me;
            for (auto i = 0u; i < m.players.size(); ++i) {
                current_state.players[i].team = m.players[i].team;
                current_state.players[i].occupied = m.players[i].occupied;
            }
        },
        [&](const auto&) {
            std::cout << "Bad message from server " << conn->get_endpoint() << std::endl;
        },
    }, msg);
}

void lobby_client::on_receive(channel::actions, const connection_ptr& conn, std::istream& data) {
    auto msg = receive_message(data);

    std::visit(ember::utility::overload {
        [&](const message::game_started& m) {
            current_state.game_start = m;
        },
        [&](const auto&) {
            std::cout << "Bad message from server " << conn->get_endpoint() << std::endl;
        },
    }, msg);
}

/* game_client */

game_client::game_client(std::shared_ptr<game_client_context> context, const lobby_state& lobby)
    : context(context), current_state(), predicted_state() {
    current_state.me = lobby.me;
    for (auto i = 0u; i < lobby.players.size(); ++i) {
        current_state.players[i].team = lobby.players[i].team;
        current_state.players[i].present = lobby.players[i].occupied;
    }
}

auto game_client::get_state() -> const game_state& {
    return predicted_state;
}

void game_client::tick([[maybe_unused]] float delta, const glm::vec2& input_dir, const glm::vec2& direction) {
    if (context->connection && current_state.me) {
        auto msg = message::player_move{};
        msg.time = predicted_state.time;
        msg.input = input_dir;
        msg.direction = direction;
        send_message<channel::state_updates>(context->connection, msg);
    }

    predicted_state.time += delta * 60.0;

    // physics
    for (auto& player : predicted_state.players) {
        if (player.present) {
            player.position += player.velocity * delta * (player.carrying ? .5f : 1.0f);

            if (player.position.x > 59.5) {
                player.position.x = 59.5;
            }
            if (player.position.x < 0.5) {
                player.position.x = 0.5;
            }
            if (player.position.y > 29.5) {
                player.position.y = 29.5;
            }
            if (player.position.y < 0.5) {
                player.position.y = 0.5;
            }

            if (player.carrying) {
                predicted_state.objective.position = player.position;
            }
        }
    }

    for (auto& kunai : predicted_state.projectiles) {
        if (kunai.state == kunai_state::FLYING) {
            kunai.position += kunai.velocity * delta;
        }
    }

    context->context.poll_events(*this);
}

void game_client::fire(const glm::vec2& direction, team_name team) {
    auto msg = message::player_fire{};
    msg.direction = direction;
    msg.team = team;
    send_message<channel::actions>(context->connection, msg);
}

void game_client::on_connect(const connection_ptr& conn) {
    std::cout << "Connected." << std::endl;
    context->connection = conn;
}

void game_client::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected. Reason: " << ec.category().name() << ": " << ec.message() << std::endl;
    current_state.me = std::nullopt;
    context->connection = nullptr;
}

void game_client::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) {
    auto msg = receive_message(data);

    std::visit(ember::utility::overload {
        [&](const message::game_state_update& m) {
            current_state.time = m.time;
            for (int i = 0; i < 4; ++i) {
                if (m.players[i].present) {
                    current_state.players[i].present = true;
                    current_state.players[i].alive = m.players[i].alive;
                    current_state.players[i].carrying = m.players[i].carrying;
                    current_state.players[i].team = m.players[i].team;
                    current_state.players[i].position = m.players[i].position;
                    current_state.players[i].velocity = m.players[i].velocity;
                } else {
                    current_state.players[i].present = false;
                }
            }
            for (int i = 0; i < m.projectiles.size(); ++i) {
                current_state.projectiles[i].state = m.projectiles[i].state;
                current_state.projectiles[i].color = m.projectiles[i].color;
                current_state.projectiles[i].team = m.projectiles[i].team;
                current_state.projectiles[i].position = m.projectiles[i].position;
                current_state.projectiles[i].velocity = m.projectiles[i].velocity;
            }
            current_state.objective.position = m.objective.position;
            current_state.me = m.me;
            predicted_state = current_state;
        },
        [&](const auto&) {
            std::cout << "Bad message from server " << conn->get_endpoint() << std::endl;
        },
    }, msg);
}

void game_client::on_receive(channel::actions, const connection_ptr& conn, std::istream& data) {}

} // namespace client
