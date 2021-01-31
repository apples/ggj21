#include "scene_lobby.hpp"

#include "scene_gameplay.hpp"

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
    auto starting = context.tick(delta);

    if (starting) {
        engine->queue_transition<scene_gameplay>(false, &context.get_state(), server_handle, renderer, context.get_context(), starting->world_size);
    }
}

void scene_lobby::render() {
    const auto& state = context.get_state();

    renderer->begin();

    for (auto i = 0u; i < state.players.size(); ++i) {
        auto& p = state.players[i];

        if (p.occupied) {
            renderer->draw_sprite("ninji", {-7.5f + 5.f * i, 0}, true, p.team == team_name::WHITE);
        }
    }

    renderer->finish();
}

auto scene_lobby::handle_game_input(const SDL_Event& event) -> bool {
    switch (event.type) {
        case SDL_EventType::SDL_KEYDOWN: {
            if (event.key.repeat == 0) {
                switch (event.key.keysym.scancode) {
                    case SDL_Scancode::SDL_SCANCODE_F1: {
                        context.start_game();
                        return true;
                    }
                }
            }
            break;
        }
    }

    return false;
}

auto scene_lobby::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_lobby.root", gui_state, engine->lua.create_table());
}
