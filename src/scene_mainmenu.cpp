#include "scene_mainmenu.hpp"

#include "scene_lobby.hpp"
#include "server.hpp"
#include "renderer.hpp"

#include "ember/engine.hpp"
#include "ember/vdom.hpp"

scene_mainmenu::scene_mainmenu(ember::engine& engine, ember::scene* prev)
    : scene(engine), gui_state{engine.lua.create_table()} {
    gui_state["start_game"] = [this]{ start_game(); };
    gui_state["join_game"] = [this](const std::string& addr){ join_game(addr); };
}

void scene_mainmenu::init() {}

void scene_mainmenu::tick(float delta) {}

void scene_mainmenu::render() {}

auto scene_mainmenu::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_mainmenu::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_mainmenu.root", gui_state, engine->lua.create_table());
}

void scene_mainmenu::start_game() {
    auto renderer = std::make_shared<game_renderer>(engine->display);
    auto server = std::make_shared<server::game_server>(*engine->io, 6969);
    server->start();
    std::cout << "Server started: " << server->get_endpoint() << std::endl;
    auto server_addr = asio::ip::udp::endpoint{asio::ip::make_address_v6("::1"), server->get_endpoint().port()};
    auto context = std::make_shared<client::game_client_context>(*engine->io, server_addr);
    context->start();
    engine->queue_transition<scene_lobby>(false, server, renderer, context);
}

void scene_mainmenu::join_game(const std::string& addr_str) {
    auto renderer = std::make_shared<game_renderer>(engine->display);
    auto addr = asio::ip::make_address_v6(addr_str);
    auto server_endpoint = asio::ip::udp::endpoint{addr, 6969};
    auto context = std::make_shared<client::game_client_context>(*engine->io, server_endpoint);
    context->start();
    engine->queue_transition<scene_lobby>(false, nullptr, renderer, context);
}
