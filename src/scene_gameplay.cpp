#include "scene_gameplay.hpp"

#include "ember/engine.hpp"
#include "ember/utility.hpp"
#include "ember/vdom.hpp"

scene_gameplay::scene_gameplay(
    ember::engine& engine,
    ember::scene* prev,
    std::shared_ptr<void> server_handle,
    const asio::ip::udp::endpoint& server_addr)
    : scene(engine),
      gui_state{engine.lua.create_table()},
      server_handle(server_handle),
      context(*engine.io, server_addr),
      sprite_shader("data/shaders/sprite.vert", "data/shaders/sprite.frag"),
      sprite_mesh(),
      background_mesh(),
      player_sprite(sushi::load_texture_2d("data/textures/ninji.png", false, false, false, false)),
      background_shader("data/shaders/sprite.vert", "data/shaders/background1.frag") {

    // build sprite mesh
    {
        auto mb = sushi::mesh_group_builder();
        mb.enable(sushi::attrib_location::POSITION);
        mb.enable(sushi::attrib_location::TEXCOORD);

        mb.mesh("sprite");

        auto ul = mb.vertex().position({-0.5, 0.5, 0}).texcoord({0, 0}).get();
        auto ur = mb.vertex().position({0.5, 0.5, 0}).texcoord({1, 0}).get();
        auto bl = mb.vertex().position({-0.5, -0.5, 0}).texcoord({0, 1}).get();
        auto br = mb.vertex().position({0.5, -0.5, 0}).texcoord({1, 1}).get();

        mb.tri(ul, ur, br);
        mb.tri(br, bl, ul);

        sprite_mesh = mb.get();
    }

    // set up camera
    camera.aspect_ratio = engine.display.aspect_ratio;
    camera.far = 10;
    camera.near = -10;
    camera.height = 10;
}

void scene_gameplay::init() {
    context.start();
}

void scene_gameplay::tick(float delta) {
    const auto& state = context.get_state();

    auto input_dir = glm::vec2{0, 0};

    if (state.me) {
        auto keys = SDL_GetKeyboardState(nullptr);

        input_dir.x -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_A]));
        input_dir.x += int(bool(keys[SDL_Scancode::SDL_SCANCODE_D]));
        input_dir.y -= int(bool(keys[SDL_Scancode::SDL_SCANCODE_S]));
        input_dir.y += int(bool(keys[SDL_Scancode::SDL_SCANCODE_W]));
    }

    context.tick(delta, input_dir);
}

void scene_gameplay::render() {
    const auto& state = context.get_state();

    if (state.me) {
        const auto& me = state.players[*state.me];
        camera.pos = -glm::vec3{me.position, 0};
    }

    auto proj = get_proj(camera);
    auto view = get_view(camera);
    auto projview = proj * view;

    for (int i = 0; i < state.players.size(); ++i) {
        auto& p = state.players[i];
        if (p.present) {
            auto model = glm::translate(glm::mat4(1), glm::vec3{p.position, 0});

            sprite_shader.bind();
            sprite_shader.set_MVP(projview * model);
            sprite_shader.set_s_texture(0);
            sprite_shader.set_tint({1, 1, 1, 1});
            sprite_shader.set_uvmat(glm::mat3(1));

            sushi::set_texture(0, player_sprite);
            sushi::draw_mesh(sprite_mesh);
        }
    }
    
    {
        auto model = glm::mat4(1);
        model = glm::translate(model, -camera.pos);
        model = glm::scale(model, glm::vec3{camera.height * camera.aspect_ratio, camera.height, 1});

        background_shader.bind();
        background_shader.set_MVP(projview * model);
        background_shader.set_modelmat(model);
        background_shader.set_time(state.time);
        background_shader.set_worldsize({100, 100});

        sushi::draw_mesh(sprite_mesh);
    }
}

auto scene_gameplay::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_gameplay::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_gameplay.root", gui_state, engine->lua.create_table());
}
