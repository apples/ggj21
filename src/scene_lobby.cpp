#include "scene_lobby.hpp"

#include "ember/engine.hpp"
#include "ember/utility.hpp"
#include "ember/vdom.hpp"

scene_lobby::scene_lobby(
    ember::engine& engine,
    ember::scene* prev,
    std::shared_ptr<void> server_handle,
    std::shared_ptr<game_renderer> renderer,
    std::shared_ptr<client::game_client_context> context)
    : scene(engine),
      server_handle(server_handle),
      renderer(renderer),
      context(context),
      gui_state{engine.lua.create_table()} {}

void scene_lobby::init() {}

void scene_lobby::tick(float delta) {
    context.tick(delta);
}

void scene_lobby::render() {
    const auto& state = context.get_state();
}

auto scene_lobby::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_lobby::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_lobby.root", gui_state, engine->lua.create_table());
}
