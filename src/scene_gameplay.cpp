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
      last_mb(0),
      anim_timer(0.f) {}

void scene_gameplay::init() {
    renderer->play_bgm("UNS, Shinobi (mperezsounds).ogg");
}

void scene_gameplay::tick(float delta) {
    const auto& state = context.get_state();

    auto input_dir = glm::vec2{0, 0};
    auto direction = glm::vec2{0, 0};

    if (state.me) {
        auto keys = SDL_GetKeyboardState(nullptr);

        input_dir.x -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_A]));
        input_dir.x += int(bool(keys[SDL_Scancode::SDL_SCANCODE_D]));
        input_dir.y -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_S]));
        input_dir.y += int(bool(keys[SDL_Scancode::SDL_SCANCODE_W]));

        if (input_dir != glm::vec2{0, 0}) {
            input_dir = glm::normalize(input_dir);
        }

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
            std::string filename = "KunaiThrow_0";
            filename.append(std::to_string(rand() % 6));
            filename.append(".ogg");
            renderer->play_sfx(filename, state.players[*state.me].position);
        }

        if(bool(keys[SDL_Scancode::SDL_SCANCODE_LSHIFT])) {
            input_dir.x /= 2;
            input_dir.y /= 2;
        }

        if(glm::length(input_dir) > .55f && int(anim_timer * 6) % 4 == 0) {
            std::string filename = "GrassStep_0";
            filename.append(std::to_string(rand() % 9));
            filename.append(".ogg");
            renderer->play_sfx(filename, state.players[*state.me].position);
        }
    }

    gui_state["score_black"] = state.score_black;
    gui_state["score_white"] = state.score_white;

    context.tick(delta, input_dir, direction);

    anim_timer += delta;
    
    if (state.me) {
        renderer->update_3d_audio(state.players[*state.me].position);
    }

    for (auto& m : context.get_sfx_queue()) {
        if (m.position) {
            renderer->play_sfx(m.name, *m.position);
        } else {
            renderer->play_sfx(m.name);
        }
    }
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

    auto anim_frame = int(anim_timer * 6.f);

    for (int i = 0; i < state.players.size(); ++i) {
        auto& p = state.players[i];
        if (p.present && p.alive) {
            auto sprite_name = glm::length(p.velocity) > 0.1 ? std::string("ninji_walk000") + std::to_string(anim_frame % 4 + 1) : "ninji";
            auto sprite_scale = p.velocity.x > 0.f ? glm::vec2{1, 1} : glm::vec2{-1, 1};
            renderer->draw_sprite(sprite_name, p.position, {0.f, 0.f}, sprite_scale, me && me->team == p.team, p.team == team_name::WHITE);
        }
    }

    for (int i = 0; i < state.projectiles.size(); ++i) {
        auto& p = state.projectiles[i];
        if (p.state == kunai_state::FLYING || p.state == kunai_state::ON_FLOOR) {
            renderer->draw_sprite(
                "BasicKunaiW", p.position, p.velocity, {1.f, 0.5f}, false, p.color == team_name::WHITE);
        }
    }

    renderer->draw_sprite("bagua", state.objective.position, true, true);
    renderer->draw_sprite("pedestal", state.objective.goal_black, true, true);
    renderer->draw_sprite("pedestal", state.objective.goal_white, true, true);

    renderer->finish();
}

auto scene_gameplay::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_gameplay::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_gameplay.root", gui_state, engine->lua.create_table());
}
