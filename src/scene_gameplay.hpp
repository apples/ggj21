#pragma once

#include "client.hpp"
#include "renderer.hpp"

#include "ember/camera.hpp"
#include "ember/scene.hpp"

#include <asio.hpp>
#include <sol.hpp>

class scene_gameplay final : public ember::scene {
public:
    scene_gameplay(
        ember::engine& eng,
        ember::scene* prev,
        const client::lobby_state* lobby,
        std::shared_ptr<void> server_handle,
        std::shared_ptr<game_renderer> renderer,
        std::shared_ptr<client::game_client_context> context,
        const glm::vec2& world_size);

    virtual void init() override;
    virtual void tick(float delta) override;
    virtual void render() override;
    virtual auto handle_game_input(const SDL_Event& event) -> bool override;
    virtual auto render_gui() -> sol::table override;

private:
    std::shared_ptr<void> server_handle;
    std::shared_ptr<game_renderer> renderer;
    client::game_client context;
    sol::table gui_state;

    glm::vec2 world_size;
    std::uint32_t last_mb;
};
