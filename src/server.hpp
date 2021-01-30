#pragma once

#include "common.hpp"
#include "channels.hpp"

#include <glm/glm.hpp>

#include <array>

namespace server {

using server_context = apply_channels_t<trellis::server_context>;
using connection_ptr = server_context::connection_ptr;

struct player_info {
    connection_ptr conn = {};
    team_name team = team_name::BLACK;
    glm::vec2 position = {0, 0};
    glm::vec2 velocity = {0, 0};
};

struct kunai_info {
    bool active = false;
    team_name team = team_name::BLACK;
    glm::vec2 position = {0, 0};
    glm::vec2 velocity = {0, 0};
};

struct game_state {
    int time = 0;
    std::array<player_info, 4> players;
    std::array<kunai_info, 4> projectiles;//4 is arbitrary here, change
};

class game_server {
public:
    game_server(asio::io_context& io, std::uint16_t port);

    ~game_server();

    auto get_endpoint() -> const asio::ip::udp::endpoint& {
        return endpoint;
    }

    void start();

    void stop();

    void on_connect(const connection_ptr& conn);
    void on_disconnect(const connection_ptr& conn, asio::error_code ec);
    void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data);

private:
    void schedule_tick();

    void tick(float delta);

    asio::io_context* io;
    server_context context;
    asio::ip::udp::endpoint endpoint;
    asio::steady_timer timer;
    asio::steady_timer::duration tick_interval;
    std::shared_ptr<void> timer_guard;

    game_state current_state;
};

} // namespace server
