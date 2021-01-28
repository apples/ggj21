#pragma once

#include "channels.hpp"

class game_server {
public:
    using server_context = apply_channels_t<trellis::server_context>;
    using connection_ptr = server_context::connection_ptr;

    game_server(asio::io_context& io, std::uint16_t port);

    ~game_server();

    auto get_endpoint() -> const asio::ip::udp::endpoint& {
        return endpoint;
    }

    void start();

    void stop();

    void on_connect(const connection_ptr& conn);
    void on_disconnect(const connection_ptr& conn, asio::error_code ec);
    void on_receive(channel::derp, const connection_ptr& conn, std::istream& data);

private:
    void schedule_tick();

    void tick();

    asio::io_context* io;
    server_context context;
    asio::ip::udp::endpoint endpoint;
    asio::steady_timer timer;
    asio::steady_timer::duration tick_interval;
    std::shared_ptr<void> timer_guard;
};
