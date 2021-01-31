#pragma once

#include "common.hpp"
#include "channels.hpp"

#include <glm/glm.hpp>

#include <array>
#include <random>

namespace server {

using server_context = apply_channels_t<trellis::server_context>;
using connection_ptr = server_context::connection_ptr;

struct player_info {
    connection_ptr conn = {};
    team_name team = team_name::BLACK;
    glm::vec2 position = {0, 0};
    glm::vec2 velocity = {0, 0};
    glm::vec2 direction = {0, 0};
    bool alive = true;
    std::array<int, 2> kunaiIds = {{-1, -1}};
};

struct kunai_info {
    kunai_state state = kunai_state::ON_FLOOR;
    team_name color = team_name::BLACK;
    std::optional<team_name> team = std::nullopt;
    glm::vec2 position = {0, 0};
    glm::vec2 velocity = {0, 0};
    glm::vec2 direction = {0, 0};
    float dist_travelled = 0.f;
};

struct game_state {
    int time = 0;
    std::array<player_info, 4> players;
    std::array<kunai_info, 8> projectiles;
};

struct lobby_player_info {
    connection_ptr conn = {};
    team_name team = team_name::BLACK;
};

struct lobby_state {
    int time = 0;
    std::array<lobby_player_info, 4> players;
};

class game_server;

class game_server_scene {
public:
    game_server_scene(game_server& server);

    virtual ~game_server_scene() = 0;

    virtual void tick(float delta) = 0;

    virtual void on_connect(const connection_ptr& conn) = 0;
    virtual void on_disconnect(const connection_ptr& conn, asio::error_code ec) = 0;
    virtual void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) = 0;
    virtual void on_receive(channel::actions, const connection_ptr& conn, std::istream& data) = 0;

protected:
    game_server* server;
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

    template <typename T, typename... Args, typename = std::enable_if_t<std::is_base_of_v<game_server_scene, T>>>
    void queue_transition(Args&&... args);

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
    std::function<std::shared_ptr<game_server_scene>(game_server& eng)> queued_transition;
};

class scene_lobby : public game_server_scene {
public:
    scene_lobby(game_server& server);

    virtual void tick(float delta) override;

    virtual void on_connect(const connection_ptr& conn) override;
    virtual void on_disconnect(const connection_ptr& conn, asio::error_code ec) override;
    virtual void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) override;
    virtual void on_receive(channel::actions, const connection_ptr& conn, std::istream& data) override;

private:
    lobby_state current_state;
};

class scene_gameplay : public game_server_scene {
public:
    scene_gameplay(game_server& server, const lobby_state* ls);

    virtual void tick(float delta) override;

    virtual void on_connect(const connection_ptr& conn) override;
    virtual void on_disconnect(const connection_ptr& conn, asio::error_code ec) override;
    virtual void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data) override;
    virtual void on_receive(channel::actions, const connection_ptr& conn, std::istream& data) override;

private:
    game_state current_state;
    glm::vec2 world_size;
    std::mt19937 rng;
};

template <typename T, typename... Args, typename>
void game_server::queue_transition(Args&&... args) {
    queued_transition = [=](game_server& eng) { return std::make_shared<T>(eng, args...); };
}

} // namespace server
