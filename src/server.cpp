#include "server.hpp"

game_server::game_server(asio::io_context& io, std::uint16_t port)
    : io(&io),
      context(io),
      endpoint({asio::ip::udp::v6(), port}),
      timer(io),
      tick_interval(
          std::chrono::duration_cast<asio::steady_timer::duration>(std::chrono::duration<double>(1.0 / 60.0))),
      timer_guard(std::make_shared<int>(0)) {}

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
            tick();
        }
    });
}

void game_server::tick() {
    context.poll_events(*this);
}

void game_server::on_connect(const connection_ptr& conn) {
    std::cout << "Connection: " << conn->get_endpoint() << std::endl;
}

void game_server::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected: " << conn->get_endpoint() << " (" << ec.category().name() << ": " << ec.message() << ")" << std::endl;
}

void game_server::on_receive(channel::derp, const connection_ptr& conn, std::istream& data) {

}
