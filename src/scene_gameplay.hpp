#pragma once

#include "ember/camera.hpp"
#include "ember/scene.hpp"

#include "channels.hpp"

#include <asio.hpp>
#include <sol.hpp>

#include "sprite_shader.hpp"

class scene_gameplay final : public ember::scene {
public:
    using client_context = apply_channels_t<trellis::client_context>;
    using connection_ptr = client_context::connection_ptr;

    scene_gameplay(ember::engine& eng, ember::scene* prev, std::shared_ptr<void> server_handle, const asio::ip::udp::endpoint& server_addr);

    virtual void init() override;
    virtual void tick(float delta) override;
    virtual void render() override;
    virtual auto handle_game_input(const SDL_Event& event) -> bool override;
    virtual auto render_gui() -> sol::table override;

    void on_connect(const connection_ptr& conn);
    void on_disconnect(const connection_ptr& conn, asio::error_code ec);
    void on_receive(channel::derp, const connection_ptr& conn, std::istream& data);

private:
    sol::table gui_state;
    std::shared_ptr<void> server_handle;
    asio::ip::udp::endpoint server_addr;
    client_context context;

    sprite_shader_program sprite_shader;
    sushi::mesh_group sprite_mesh;
    sushi::texture_2d player_sprite;

    ember::camera::orthographic camera;
};
