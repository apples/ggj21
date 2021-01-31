#pragma once

#include "sprite_shader.hpp"
#include "background_shader.hpp"

#include "ember/camera.hpp"
#include "ember/display.hpp"

#include <sushi/sushi.hpp>
#include <glm/glm.hpp>

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <unordered_map>
#include <string>

class game_renderer {
public:
    game_renderer(const display_info& display);

    void set_camera_pos(const glm::vec2& pos);

    void begin();

    void draw_background(int time, const glm::vec2& world_size);

    void draw_sprite(const std::string& name, const glm::vec2& pos, bool my_team, bool white);

    void draw_sprite(const std::string& name, const glm::vec2& pos, const glm::vec2& facing, const glm::vec2& scale, bool my_team, bool white);

    void finish();

    void update_3d_audio(const glm::vec2& pos);

    void play_sfx(const std::string& name);
    void play_sfx(const std::string& name, const glm::vec2& pos);
    void play_bgm(const std::string& name);

private:
    sprite_shader_program sprite_shader;
    background_shader_program background_shader;
    sushi::mesh_group sprite_mesh;
    std::unordered_map<std::string, sushi::texture_2d> sprite_textures;
    
    SoLoud::Soloud soloud;
    std::unordered_map<std::string, SoLoud::Wav> sfx;
    std::unordered_map<std::string, SoLoud::WavStream> bgm;
    std::optional<SoLoud::handle> bgm_handle;

    ember::camera::orthographic camera;
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 projview;
};
