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
        if (kunai.state == kunai_state::FLYING) {
            auto adv = kunai.velocity * delta;
            kunai.position += adv;
            kunai.dist_travelled += glm::length(adv);

            if (kunai.dist_travelled > 10.f) {
                kunai.state = kunai_state::ON_FLOOR;
            }
        }
    }

    //collision
    auto collides_with = [](const auto& p1, float r1, const auto& p2, float r2) {
        return glm::length(p1 - p2) <= (r1 + r2);
    };

    for(auto i = 0u; i < current_state.projectiles.size(); ++i) {
        auto& kunai = current_state.projectiles[i];

        if (kunai.state == kunai_state::FLYING) {
            for(auto& kunai2 : current_state.projectiles) {
                if (&kunai == &kunai2) continue;

                if (kunai2.state == kunai_state::FLYING) {
                    if (collides_with(kunai.position, 0.25f, kunai2.position, 0.25f)) {
                        kunai.state = kunai_state::ON_FLOOR;
                        kunai.velocity = {0, 0};
                        kunai.team = std::nullopt;
                        kunai2.state = kunai_state::ON_FLOOR;
                        kunai2.velocity = {0, 0};
                        kunai2.team = std::nullopt;
                    }
                }
            }
        }

        for(auto& player : current_state.players) {
            if (player.conn) {
                switch (kunai.state) {
                    case kunai_state::FLYING: {
                        if (kunai.team != player.team && collides_with(kunai.position, 0.25, player.position, 0.5f)) {
                            player.alive = false;
                            std::cout << "Hit!" << std::endl;
                        }
                        break;
                    }
                    case kunai_state::ON_FLOOR: {
                        if (collides_with(kunai.position, 0.25, player.position, 0.5f)) {
                            if (player.kunaiIds[0] == -1) {
                                player.kunaiIds[0] = i;
                                kunai.state = kunai_state::HELD;
                            } else if (player.kunaiIds[1] == -1) {
                                player.kunaiIds[1] = i;
                                kunai.state = kunai_state::HELD;
                            }
                        }
                        break;
                    }
                }
            }
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
            update.projectiles[i].state = current_state.projectiles[i].state;
            update.projectiles[i].color = current_state.projectiles[i].color;
            update.projectiles[i].team = current_state.projectiles[i].team;
            update.projectiles[i].position = current_state.projectiles[i].position;
            update.projectiles[i].velocity = current_state.projectiles[i].velocity;
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
                        auto id = std::exchange(player.kunaiIds[1], -1);

                        if (id == -1) {
                            id = std::exchange(player.kunaiIds[0], -1);
                        }

                        if (id != -1) {
                            auto& newKunai = current_state.projectiles[id];
                            newKunai.state = kunai_state::FLYING;
                            newKunai.team = player.team;
                            newKunai.direction = m.direction;
                            newKunai.position = player.position;
                            newKunai.velocity = m.direction * 20.0f;
                            newKunai.dist_travelled = 0.f;
                        }
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
