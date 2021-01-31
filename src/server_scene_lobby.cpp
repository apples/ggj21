#include "server.hpp"

namespace server {

scene_lobby::scene_lobby(game_server& server) : game_server_scene(server) {
    current_state.players[1].team = team_name::WHITE;
    current_state.players[3].team = team_name::WHITE;
}

void scene_lobby::tick(float delta) override {
    // send state to clients
    {
        auto update = message::lobby_state_update{};

        update.time = current_state.time;

        for (auto i = 0u; i < 4; ++i) {
            if (current_state.players[i].conn) {
                update.players[i].occupied = true;
                update.players[i].team = current_state.players[i].team;
            } else {
                update.players[i].present = false;
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

void scene_lobby::on_connect(const connection_ptr& conn) override {
    std::cout << "Connection: " << conn->get_endpoint() << std::endl;

    for (auto& player : current_state.players) {
        if (!player.conn) {
            player.conn = conn;
            return;
        }
    }

    std::cout << "Server full!" << std::endl;

    conn->disconnect();
}

void scene_lobby::on_disconnect(const connection_ptr& conn, asio::error_code ec) override {
    std::cout << "Disconnected: " << conn->get_endpoint() << " (" << ec.category().name() << ": " << ec.message()
                << ")" << std::endl;

    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            player.conn = nullptr;
            return;
        }
    }
}

void scene_lobby::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) override {
    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            auto msg = receive_message(data);

            std::visit(
                ember::utility::overload{
                    [&](const auto&) {
                        std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
                        conn->disconnect();
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

void scene_lobby::on_receive(channel::actions, const connection_ptr& conn, std::istream& data) override {
    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            auto msg = receive_message(data);

            std::visit(
                ember::utility::overload{
                    [&](const message::lobby_flip_color& m) {
                        player.team = (player.team == team_name::BLACK ? team_name::WHITE : team_name::BLACK);
                    },
                    [&](const message::lobby_start& m) {
                        server->queue_transition<scene_gameplay>(&current_state);
                    },
                    [&](const auto&) {
                        std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
                        conn->disconnect();
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
