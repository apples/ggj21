#pragma once

#include "ember/camera.hpp"
#include "ember/scene.hpp"

#include "client.hpp"

#include <asio.hpp>
#include <sol.hpp>

#include "sprite_shader.hpp"
#include "background_shader.hpp"

class scene_gameplay final : public ember::scene {
public:
    scene_gameplay(ember::engine& eng, ember::scene* prev, std::shared_ptr<void> server_handle, const asio::ip::udp::endpoint& server_addr);

    virtual void init() override;
    virtual void tick(float delta) override;
    virtual void render() override;
    virtual auto handle_game_input(const SDL_Event& event) -> bool override;
    virtual auto render_gui() -> sol::table override;

private:
    sol::table gui_state;
    std::shared_ptr<void> server_handle;
    client::game_client context;

    sprite_shader_program sprite_shader;
    sushi::mesh_group sprite_mesh;
    sushi::texture_2d player_sprite;
    background_shader_program background_shader;
    sushi::mesh_group background_mesh;

    ember::camera::orthographic camera;
};
