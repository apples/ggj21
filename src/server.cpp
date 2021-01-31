#include "server.hpp"

#include "messages.hpp"

#include "ember/utility.hpp"

namespace server {

game_server_scene::game_server_scene(game_server& server) : server(&server) {}

game_server::game_server(asio::io_context& io, std::uint16_t port)
    : io(&io),
      context(io),
      endpoint({asio::ip::udp::v6(), port}),
      timer(io),
      tick_interval(
          std::chrono::duration_cast<asio::steady_timer::duration>(std::chrono::duration<double>(1.0 / 60.0))),
      timer_guard(std::make_shared<int>(0)),
      scene(std::make_shared<scene_lobby>(*this)) {}

game_server::~game_server() { stop(); }

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
        if (ec)
            return;
        if (auto ptr = wptr.lock()) {
            schedule_tick();
            tick(1.0 / 60.0);
        }
    });
}

void game_server::tick(float delta) {
    context.poll_events(*scene);
    scene->tick(delta);
}

} // namespace server
