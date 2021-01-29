#include "scene_mainmenu.hpp"

#include "scene_gameplay.hpp"
#include "server.hpp"

#include "ember/engine.hpp"
#include "ember/vdom.hpp"

scene_mainmenu::scene_mainmenu(ember::engine& engine, ember::scene* prev)
    : scene(engine), gui_state{engine.lua.create_table()} {
    gui_state["start_game"] = [this]{ start_game(); };
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
    auto server = std::make_shared<server::game_server>(*engine->io, 6969);
    server->start();
    std::cout << "Server started: " << server->get_endpoint() << std::endl;
    auto server_addr = asio::ip::udp::endpoint{asio::ip::make_address_v6("::1"), server->get_endpoint().port()};
    engine->queue_transition<scene_gameplay>(false, server, server_addr);
}
