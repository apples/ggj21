#include "scene_gameplay.hpp"

#include "ember/engine.hpp"
#include "ember/utility.hpp"
#include "ember/vdom.hpp"

scene_gameplay::scene_gameplay(
    ember::engine& engine,
    ember::scene* prev,
    const client::lobby_state* lobby,
    std::shared_ptr<void> server_handle,
    std::shared_ptr<game_renderer> renderer,
    std::shared_ptr<client::game_client_context> context,
    const glm::vec2& world_size)
    : scene(engine),
      server_handle(server_handle),
      renderer(renderer),
      context(context, *lobby),
      gui_state{engine.lua.create_table()},
      world_size(world_size),
      last_mb(0) {}

void scene_gameplay::init() {}

//collision
auto collides_with = [](const auto& p1, float r1, const auto& p2, float r2) {
    return glm::length(p1 - p2) <= (r1 + r2);
};

void scene_gameplay::tick(float delta) {
    const auto& state = context.get_state();

    auto input_dir = glm::vec2{0, 0};
    auto direction = glm::vec2{0, 0};
    bool carrying = false;

    if (state.me) {
        auto keys = SDL_GetKeyboardState(nullptr);

        input_dir.x -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_A]));
        input_dir.x += int(bool(keys[SDL_Scancode::SDL_SCANCODE_D]));
        input_dir.y -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_S]));
        input_dir.y += int(bool(keys[SDL_Scancode::SDL_SCANCODE_W]));

        int x;
        int y;
        auto mb = SDL_GetMouseState(&x, &y);
        auto mb_pressed = mb & ~last_mb;
        [[maybe_unused]] auto mb_released = ~mb & last_mb;

        last_mb = mb;

        direction = glm::normalize(
            glm::vec2{x - engine->display.width / 2, engine->display.height / 2 - y});

        if (mb_pressed & SDL_BUTTON_LMASK && state.players[*state.me].alive) {
            context.fire(direction, state.players[*state.me].team);
        }

        carrying = bool(keys[SDL_Scancode::SDL_SCANCODE_SPACE]) && collides_with(state.players[*state.me].position, .5f, state.objective.position, 1.0f);
    }

    context.tick(delta, input_dir, direction, carrying);
}

void scene_gameplay::render() {
    const auto& state = context.get_state();

    auto me = (const client::player_info*)nullptr;

    if (state.me) {
        me = &state.players[*state.me];
        renderer->set_camera_pos(me->position);
    }

    renderer->begin();

    renderer->draw_background(state.time, world_size);

    for (int i = 0; i < state.players.size(); ++i) {
        auto& p = state.players[i];
        if (p.present && p.alive) {
            renderer->draw_sprite("ninji", p.position, me && me->team == p.team, p.team == team_name::WHITE);
        }
    }

    for (int i = 0; i < state.projectiles.size(); ++i) {
        auto& p = state.projectiles[i];
        if (p.state == kunai_state::FLYING || p.state == kunai_state::ON_FLOOR) {
            renderer->draw_sprite(
                "BasicKunaiW", p.position, p.velocity, false, p.color == team_name::WHITE);
        }
    }

    renderer->draw_sprite(
        "bagua", state.objective.position, true, true);

    renderer->finish();
}

auto scene_gameplay::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_gameplay::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_gameplay.root", gui_state, engine->lua.create_table());
}
