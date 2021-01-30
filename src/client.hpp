#pragma once

#include "common.hpp"
#include "channels.hpp"

#include <trellis/trellis.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace client {

using client_context = apply_channels_t<trellis::client_context>;
using connection_ptr = client_context::connection_ptr;

class game_client_context {
public:
    game_client_context(asio::io_context& io, asio::ip::udp::endpoint server_addr);

    ~game_client_context();

    void start();

    void stop();

    client_context context;
    asio::ip::udp::endpoint server_addr;
    connection_ptr connection;
};

struct player_info {
    bool present = false;
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
};

struct game_state {
    double time = 0;
    std::array<player_info, 4> players;
    std::array<kunai_info, 8> projectiles;
    std::optional<int> me = std::nullopt;
};

struct lobby_player_info {
    team_name team = team_name::BLACK;
    bool occupied;
};

struct lobby_state {
    std::optional<int> me = std::nullopt;
    std::array<lobby_player_info, 4> players;
};

class lobby_client {
public:
    lobby_client(std::shared_ptr<game_client_context> context);

    auto get_state() -> const lobby_state&;

    void tick(float delta);

    void on_connect(const connection_ptr& conn);
    void on_disconnect(const connection_ptr& conn, asio::error_code ec);
    void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data);

private:
    std::shared_ptr<game_client_context> context;

    lobby_state current_state;
};

class game_client {
public:
    game_client(std::shared_ptr<game_client_context> context);

    auto get_state() -> const game_state&;

    void tick(float delta, const glm::vec2& input_dir, const glm::vec2& direction);

    void fire(const glm::vec2& direction);

    void on_connect(const connection_ptr& conn);
    void on_disconnect(const connection_ptr& conn, asio::error_code ec);
    void on_receive(channel::state_updates, const connection_ptr& conn, std::istream& data);

private:
    std::shared_ptr<game_client_context> context;
    
    game_state current_state;
    game_state predicted_state;
};

} // namespace client
