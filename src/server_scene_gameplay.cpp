#include "server.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace server {

scene_gameplay::scene_gameplay(game_server& server, const lobby_state* ls, const glm::vec2& world_size)
    : game_server_scene(server), world_size(world_size), rng(std::random_device{}()) {
    for (auto i = 0u; i < ls->players.size(); ++i) {
        current_state.players[i].conn = ls->players[i].conn;
        current_state.players[i].team = ls->players[i].team;

        switch(i) {
            case 0u:
                current_state.players[i].position = glm::vec2(2, 13);
                break;
            case 1u:
                current_state.players[i].position = glm::vec2(58, 13);
                break;
            case 2u:
                current_state.players[i].position = glm::vec2(2, 17);
                break;
            case 3u:
                current_state.players[i].position = glm::vec2(58, 17);
                break;
            default:
                current_state.players[i].position = glm::vec2(0, 0);
        }
    }
    
    current_state.objective.position = glm::vec2(30, 15);
    current_state.objective.goal_black = glm::vec2(2, 15);
    current_state.objective.goal_white = glm::vec2(58, 15);

    auto wsx_dist = std::uniform_real_distribution{10.f, world_size.x - 10.f};
    auto wsy_dist = std::uniform_real_distribution{10.f, world_size.y - 10.f};

    bool w = false;

    for (auto& k : current_state.projectiles) {
        k.position = {wsx_dist(rng), wsy_dist(rng)};
        k.color = w ? team_name::WHITE : team_name::BLACK;
        w = !w;
    }
}

void scene_gameplay::tick(float delta) {
    // advance frame
    ++current_state.time;

    // physics
    for (auto& player : current_state.players) {
        if (player.conn) {
            player.position += player.velocity * delta * (player.carrying ? .5f : 1.0f);

            if(player.position.x > 59.5) {
                player.position.x = 59.5;
            }
            if(player.position.x < 0.5) {
                player.position.x = 0.5;
            }
            if(player.position.y > 29.5) {
                player.position.y = 29.5;
            }
            if(player.position.y < 0.5) {
                player.position.y = 0.5;
            }

            if(player.carrying) {
                current_state.objective.position = player.position;
            }

            //respawn
            if(!player.alive) {
                player.respawnTimer += delta;
                if(player.respawnTimer > 5.0f) {
                    player.alive = true;
                    player.respawnTimer = 0;
                    switch(std::distance(&current_state.players[0], &player)) {
                        case 0:
                            player.position = glm::vec2(2, 13);
                            break;
                        case 1:
                            player.position = glm::vec2(58, 13);
                            break;
                        case 2:
                            player.position = glm::vec2(2, 17);
                            break;
                        case 3:
                            player.position = glm::vec2(58, 17);
                            break;
                        default:
                            player.position = glm::vec2(0, 0);
                    }
                }
            }
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

        // kunai <=> kunai
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

        // kunai <=> player
        for(auto& player : current_state.players) {
            if (player.conn && player.alive) {
                switch (kunai.state) {
                    case kunai_state::FLYING: {
                        if (kunai.team != player.team && collides_with(kunai.position, 0.25, player.position, 0.5f)) {
                            player.alive = false;
                            player.carrying = false;
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

        // kunai <=> wall
        if (kunai.state == kunai_state::FLYING) {
            if(kunai.position.x < 0.25 ||
                kunai.position.x > 59.75) {
                    kunai.velocity = glm::vec2(-kunai.velocity.x, kunai.velocity.y);
            }
            if(kunai.position.y < 0.25 ||
                kunai.position.y > 29.75) {
                    kunai.velocity = glm::vec2(kunai.velocity.x, -kunai.velocity.y);
            }
                // kunai.state = kunai_state::ON_FLOOR;
                // kunai.velocity = {0, 0};
                // kunai.team = std::nullopt;
        }
    }

    // player <=> objective
    if (!current_state.objective.carried) {
        for (auto& player : current_state.players) {
            if (player.conn && player.alive) {
                if (collides_with(player.position, 0.5, current_state.objective.position, 0.5)) {
                    current_state.objective.carried = true;
                    player.carrying = true;
                }
            }
        }
    }

    // player <=> pedestal
    for (auto& player : current_state.players) {
        if (player.conn && player.alive) {
            auto& goal = player.team == team_name::BLACK ? current_state.objective.goal_black : current_state.objective.goal_white;
            auto& score = player.team == team_name::BLACK ? current_state.score_black : current_state.score_white;

            if (player.carrying && collides_with(player.position, 0.5, goal, 0.5)) {
                current_state.objective.carried = false;
                current_state.objective.position = glm::vec2(30, 15);
                player.carrying = false;
                ++score;
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
                update.players[i].alive = current_state.players[i].alive;
                update.players[i].carrying = current_state.players[i].carrying;
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

        update.objective.position = current_state.objective.position;
        update.objective.goal_black = current_state.objective.goal_black;
        update.objective.goal_white = current_state.objective.goal_white;

        update.score_black = current_state.score_black;
        update.score_white = current_state.score_white;

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
