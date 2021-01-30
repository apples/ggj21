#include "server.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace server {

game_server::game_server(asio::io_context& io, std::uint16_t port)
    : io(&io),
      context(io),
      endpoint({asio::ip::udp::v6(), port}),
      timer(io),
      tick_interval(
          std::chrono::duration_cast<asio::steady_timer::duration>(std::chrono::duration<double>(1.0 / 60.0))),
      timer_guard(std::make_shared<int>(0)),
      current_state() {
          current_state.players[1].team = team_name::WHITE;
          current_state.players[3].team = team_name::WHITE;
      }

game_server::~game_server() {
    stop();
}

void game_server::start() {
    context.listen(endpoint);
    schedule_tick();
}

void game_server::stop() {
    timer.cancel();
    context.stop();
}

void game_server::schedule_tick() {
    timer.expires_from_now(tick_interval);
    timer.async_wait([this, wptr = std::weak_ptr<void>(timer_guard)](asio::error_code ec) {
        if (ec) return;
        if (auto ptr = wptr.lock()) {
            schedule_tick();
            tick(1.0 / 60.0);
        }
    });
}

void game_server::tick(float delta) {
    context.poll_events(*this);

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

        for(int i = 0; i < current_state.projectiles.size(); i++) {
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

void game_server::on_connect(const connection_ptr& conn) {
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

void game_server::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected: " << conn->get_endpoint() << " (" << ec.category().name() << ": " << ec.message() << ")" << std::endl;

    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            player.conn = nullptr;
            return;
        }
    }
}

void game_server::on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) {
    for (auto& player : current_state.players) {
        if (player.conn == conn) {
            auto msg = receive_message(data);

            std::visit(ember::utility::overload {
                [&](const message::player_move& m) {
                    player.velocity = m.input * 10.f;
                    //player.position += player.velocity * (1.f / 60.f) * float(current_state.time - m.time);
                },
                [&](const message::player_fire& m) {
                    auto newKunai = server::kunai_info();
                    newKunai.active = true;
                    newKunai.direction = m.direction;
                    newKunai.position = player.position;
                    newKunai.velocity = m.direction * 5.0f;
                    current_state.projectiles[0] = newKunai;
                },
                [&](const auto&) {
                    std::cout << "Bad message from player " << conn->get_endpoint() << std::endl;
                    conn->disconnect();
                },
            }, msg);

            return;
        }
    }

    // bad client?
    std::cout << "Bad client " << conn->get_endpoint() << std::endl;
    conn->disconnect();
}

} // namespace server
