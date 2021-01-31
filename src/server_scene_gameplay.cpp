#include "server.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace server {

scene_gameplay::scene_gameplay(game_server& server, const lobby_state* ls) : game_server_scene(server) {
    for (auto i = 0u; i < ls->players.size(); ++i) {
        current_state.players[i].conn = ls->players[i].conn;
        current_state.players[i].team = ls->players[i].team;
    }
}

void scene_gameplay::tick(float delta) {
    // advance frame
    ++current_state.time;

    // physics
    for (auto& player : current_state.players) {
        if (player.conn) {
            player.position += player.velocity * delta;
        }
    }

    for (auto& kunai : current_state.projectiles) {
        if (kunai.active) {
            kunai.position += kunai.velocity * delta;
        }
    }

    // send state to clients
    {
        auto update = message::game_state_update{};

        update.time = current_state.time;

        for (auto i = 0u; i < 4; ++i) {
            if (current_state.players[i].conn) {
                update.players[i].present = true;
                update.players[i].team = current_state.players[i].team;
                update.players[i].position = current_state.players[i].position;
                update.players[i].velocity = current_state.players[i].velocity;
            } else {
                update.players[i].present = false;
            }
        }

        for (auto i = 0u; i < current_state.projectiles.size(); ++i) {
            if (current_state.projectiles[i].active) {
                update.projectiles[i].active = true;
                update.projectiles[i].team = current_state.projectiles[i].team;
                update.projectiles[i].position = current_state.projectiles[i].position;
                update.projectiles[i].velocity = current_state.projectiles[i].velocity;
            } else {
                update.projectiles[i].active = false;
            }
        }

        for (auto i = 0u; i < current_state.players.size(); ++i) {
            auto& p = current_state.players[i];
            if (p.conn) {
                update.me = i;
                send_message<channel::state_updates>(p.conn, update);
            }
        }
    }
}

void scene_gameplay::on_connect(const connection_ptr& conn) {
    std::cout << "Connection: " << conn->get_endpoint() << std::endl;

    for (auto& player : current_state.players) {
        if (!player.conn) {
            player.conn = conn;
            player.position = {0, 0};
            player.velocity = {0, 0};
            return;
        }
    }

    std::cout << "Server full!" << std::endl;

    conn->disconnect();
}

void scene_gameplay::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected: " << conn->get_endpoint() << " (" << ec.category().name() << ": " << ec.message()
                << ")" << std::endl;

    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            player.conn = nullptr;
            return;
        }
    }
}

void scene_gameplay::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) {
    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            auto msg = receive_message(data);

            std::visit(
                ember::utility::overload{
                    [&](const message::player_move& m) {
                        player.velocity = m.input * 10.f;
                        // player.position += player.velocity * (1.f / 60.f) * float(current_state.time - m.time);
                    },
                    [&](const auto&) {
                        std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
                    },
                },
                msg);

            return;
        }
    }

    // bad client?
    std::cout << "Bad client " << conn->get_endpoint() << std::endl;
    conn->disconnect();
}

void scene_gameplay::on_receive(channel::actions, const connection_ptr& conn, std::istream& data) {
    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            auto msg = receive_message(data);

            std::visit(
                ember::utility::overload{
                    [&](const message::player_fire& m) {
                        std::cout << "Player fired kunai " << m.direction.x << ", " << m.direction.y << std::endl;
                        auto newKunai = server::kunai_info{};
                        newKunai.active = true;
                        newKunai.direction = m.direction;
                        newKunai.position = player.position;
                        newKunai.velocity = m.direction * 5.0f;
                        current_state.projectiles[0] = newKunai;
                    },
                    [&](const auto&) {
                        std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
                    },
                },
                msg);

            return;
        }
    }

    // bad client?
    std::cout << "Bad client " << conn->get_endpoint() << std::endl;
    conn->disconnect();
}

} // namespace server
