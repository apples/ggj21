#include "scene_gameplay.hpp"

#include "ember/engine.hpp"
#include "ember/vdom.hpp"

scene_gameplay::scene_gameplay(
    ember::engine& engine,
    ember::scene* prev,
    std::shared_ptr<void> server_handle,
    const asio::ip::udp::endpoint& server_addr)
    : scene(engine),
      gui_state{engine.lua.create_table()},
      server_handle(server_handle),
      server_addr(server_addr),
      context(*engine.io),
      sprite_shader("data/shaders/sprite.vert", "data/shaders/sprite.frag"),
      sprite_mesh(),
      player_sprite(sushi::load_texture_2d("data/textures/ninji.png", false, false, false, false)) {
    
    // build sprite mesh
    {
        auto mb = sushi::mesh_group_builder();
        mb.enable(sushi::attrib_location::POSITION);
        mb.enable(sushi::attrib_location::TEXCOORD);

        mb.mesh("sprite");

        auto ul = mb.vertex().position({-1, 1, 0}).texcoord({0, 0}).get();
        auto ur = mb.vertex().position({1, 1, 0}).texcoord({1, 0}).get();
        auto bl = mb.vertex().position({-1, -1, 0}).texcoord({0, 1}).get();
        auto br = mb.vertex().position({1, -1, 0}).texcoord({1, 1}).get();

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
    context.connect({asio::ip::udp::v6(), 0}, server_addr);
}

void scene_gameplay::tick(float delta) {
    context.poll_events(*this);
}

void scene_gameplay::render() {
    auto proj = get_proj(camera);
    auto view = get_view(camera);
    auto model = glm::mat4(1);

    sprite_shader.bind();
    sprite_shader.set_MVP(proj * view * model);
    sprite_shader.set_s_texture(0);
    sprite_shader.set_tint({1, 1, 1, 1});
    sprite_shader.set_uvmat(glm::mat3(1));

    sushi::set_texture(0, player_sprite);
    sushi::draw_mesh(sprite_mesh);
}

auto scene_gameplay::handle_game_input(const SDL_Event& event) -> bool {
    return false;
}

auto scene_gameplay::render_gui() -> sol::table {
    return ember::vdom::create_element(engine->lua, "gui.scene_gameplay.root", gui_state, engine->lua.create_table());
}

void scene_gameplay::on_connect(const connection_ptr& conn) {
    std::cout << "Connected." << std::endl;
}

void scene_gameplay::on_disconnect(const connection_ptr& conn, asio::error_code ec) {
    std::cout << "Disconnected. Reason: " << ec.category().name() << ": " << ec.message() << std::endl;
}

void scene_gameplay::on_receive(channel::derp, const connection_ptr& conn, std::istream& data) {

}
