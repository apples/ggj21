#pragma once

#include "renderer.hpp"

#include "ember/scene.hpp"

#include <sol.hpp>

class scene_mainmenu final : public ember::scene {
public:
    scene_mainmenu(
        ember::engine& eng, ember::scene* prev, std::shared_ptr<game_renderer> renderer, std::uint16_t server_port);

    virtual void init() override;
    virtual void tick(float delta) override;
    virtual void render() override;
    virtual auto handle_game_input(const SDL_Event& event) -> bool override;
    virtual auto render_gui() -> sol::table override;

private:
    void start_game();
    void join_game(const std::string& addr_str);

    sol::table gui_state;
    std::shared_ptr<game_renderer> renderer;
    std::uint16_t server_port;
};
