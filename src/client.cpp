#include "client.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace client {

game_client::game_client(asio::io_context& io, asio::ip::udp::endpoint server_addr)
    : context(io), server_addr(server_addr), current_state(), connection() {}

game_client::~game_client() {
    stop();
}

auto game_client::get_state() -> const game_state& {
    return predicted_state;
}

void game_client::start() {
    context.connect({asio::ip::udp::v6(), 0}, server_addr);
}

void game_client::stop() {
    context.stop();
}

void game_client::tick([[maybe_unused]] float delta, const glm::vec2& input_dir) {
    if (connection && current_state.me) {
        auto msg = message::player_move{};
        msg.time = predicted_state.time;
        msg.input = input_dir;
        send_message<channel::state_updates>(connection, msg);
    }

    predicted_state.time += delta * 60.0;
    
    // physics
    for (auto& player : predicted_state.players) {
        if (player.present) {
            player.position += player.velocity * delta;
        }
    }

    for (auto& kunai : predicted_state.projectiles) {
        if (kunai.active) {
            kunai.position += kunai.velocity * delta;
        }
    }

    context.poll_events(*this);
}

void game_client::on_connect(const connection_ptr& conn) {
    std::cout << "Connected." << std::endl;
    connection = conn;
}

void game_client::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected. Reason: " << ec.category().name() << ": " << ec.message() << std::endl;
    current_state.me = std::nullopt;
    connection = nullptr;
}

void game_client::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) {
    auto msg = receive_message(data);

    std::visit(ember::utility::overload {
        [&](const message::game_state_update& m) {
            current_state.time = m.time;
            for (int i = 0; i < 4; ++i) {
                if (m.players[i].present) {
                    current_state.players[i].present = true;
                    current_state.players[i].team = m.players[i].team;
                    current_state.players[i].position = m.players[i].position;
                    current_state.players[i].velocity = m.players[i].velocity;
                } else {
                    current_state.players[i].present = false;
                }
            }
            current_state.me = m.me;
            predicted_state = current_state;
        },
        [&](const auto&) {
            std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
            conn->disconnect();
        },
    }, msg);
}

} // namespace client
