#include "scene_gameplay.hpp"

#include "ember/engine.hpp"
#include "ember/utility.hpp"
#include "ember/vdom.hpp"

scene_gameplay::scene_gameplay(
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

void scene_gameplay::init() {}

void scene_gameplay::tick(float delta) {
    const auto& state = context.get_state();

    auto input_dir = glm::vec2{0, 0};
    auto direction = glm::vec2{0, 0};
    bool fire = false;

    if (state.me) {
        auto keys = SDL_GetKeyboardState(nullptr);

        input_dir.x -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_A]));
        input_dir.x += int(bool(keys[SDL_Scancode::SDL_SCANCODE_D]));
        input_dir.y -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_S]));
        input_dir.y += int(bool(keys[SDL_Scancode::SDL_SCANCODE_W]));

        int x;
        int y;
        if(SDL_GetMouseState(&x, &y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            fire = true;
        }
        direction = glm::vec2(x / engine->display.width, y / engine->display.height);
        
    }

    context.tick(delta, input_dir, direction);
}

void scene_gameplay::render() {
    const auto& state = context.get_state();

    auto me = (const client::player_info*)nullptr;

    if (state.me) {
        me = &state.players[*state.me];
        renderer->set_camera_pos(me->position);
    }

    renderer->begin();

    renderer->draw_background(state.time, {100, 100});

    for (int i = 0; i < state.players.size(); ++i) {
        auto& p = state.players[i];
        if (p.present) {
            renderer->draw_sprite("ninji", p.position, me && me->team == p.team, p.team == team_name::WHITE);
        }
    }

    renderer->finish();
}

auto scene_gameplay::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_gameplay::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_gameplay.root", gui_state, engine->lua.create_table());
}
