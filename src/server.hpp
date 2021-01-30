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
    glm::vec2 direction = {0, 0};
};

struct kunai_info {
    bool active = false;
    team_name team = team_name::BLACK;
    glm::vec2 position = {0, 0};
    glm::vec2 velocity = {0, 0};
    glm::vec2 direction = {0, 0};
};

struct game_state {
    int time = 0;
    std::array<player_info, 4> players;
    std::array<kunai_info, 8> projectiles;
};

class game_server_scene {
public:
    game_server_scene();

    virtual ~game_server_scene() = 0;

    virtual void tick(float delta) = 0;

    virtual void on_connect(const connection_ptr& conn) = 0;
    virtual void on_disconnect(const connection_ptr& conn, asio::error_code ec) = 0;
    virtual void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) = 0;
    virtual void on_receive(channel::actions, const connection_ptr& conn, std::istream& data) = 0;
};

inline game_server_scene::~game_server_scene() = default;

class game_server {
public:
    game_server(asio::io_context& io, std::uint16_t port);

    ~game_server();

    auto get_endpoint() -> const asio::ip::udp::endpoint& {
        return endpoint;
    }

    void start();

    void stop();

private:
    void schedule_tick();

    void tick(float delta);

    asio::io_context* io;
    server_context context;
    asio::ip::udp::endpoint endpoint;
    asio::steady_timer timer;
    asio::steady_timer::duration tick_interval;
    std::shared_ptr<void> timer_guard;

    std::shared_ptr<game_server_scene> scene;
};

} // namespace server
