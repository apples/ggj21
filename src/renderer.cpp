
#include "renderer.hpp"

#include <glm/gtx/vector_angle.hpp>

#include <iostream>
#include <soloud_sfxr.h>

game_renderer::game_renderer(const display_info& display)
    : sprite_shader("data/shaders/sprite.vert", "data/shaders/sprite.frag"),
      background_shader("data/shaders/sprite.vert", "data/shaders/background1.frag"),
      sprite_mesh(),
      sprite_textures(),
      camera(),
      proj(1.f),
      view(1.f),
      projview(1.f) {

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
    camera.aspect_ratio = display.aspect_ratio;
    camera.far = 10;
    camera.near = -10;
    camera.height = 10;

    // audio
    soloud.init();
}

void game_renderer::set_camera_pos(const glm::vec2& pos) {
    camera.pos = glm::vec3{-pos, 0};
}

void game_renderer::begin() {
    proj = get_proj(camera);
    view = get_view(camera);
    projview = proj * view;
}

void game_renderer::draw_background(int time, const glm::vec2& world_size) {
    auto model = glm::mat4(1);
    model = glm::translate(model, glm::vec3{-camera.pos.x, -camera.pos.y, -1});
    model = glm::scale(model, glm::vec3{camera.height * camera.aspect_ratio, camera.height, 1});

    background_shader.bind();
    background_shader.set_MVP(projview * model);
    background_shader.set_modelmat(model);
    background_shader.set_time(time);
    background_shader.set_worldsize(world_size);

    sushi::draw_mesh(sprite_mesh);
}

void game_renderer::draw_sprite(const std::string& name, const glm::vec2& pos, bool my_team, bool white) {
    draw_sprite(name, pos, {0.f, 0.f}, {1.f, 1.f}, my_team, white);
}

void game_renderer::draw_sprite(const std::string& name, const glm::vec2& pos, const glm::vec2& facing, const glm::vec2& scale, bool my_team, bool white) {
    auto model = glm::translate(glm::mat4(1.f), glm::vec3{pos, 0});

    if (facing != glm::vec2{0.f, 0.f}) {
        auto angle = std::atan2(facing.y, facing.x);
        model = glm::rotate(model, angle, {0, 0, 1});
    }
    
    model = glm::scale(model, {scale, 1});

    sprite_shader.bind();
    sprite_shader.set_MVP(projview * model);
    sprite_shader.set_s_texture(0);
    sprite_shader.set_uvmat(glm::mat3(1));

    if (my_team) {
        sprite_shader.set_tint({1, 1, 1, 1});
        sprite_shader.set_invert(!white);
    } else {
        sprite_shader.set_tint({0, 0, 0, 0});
        sprite_shader.set_invert(white);
    }

    auto tex_iter = sprite_textures.find(name);
    if (tex_iter == sprite_textures.end()) {
        auto texture = sushi::load_texture_2d("data/textures/" + name + ".png", false, false, false, false);
        tex_iter = sprite_textures.emplace(name, std::move(texture)).first;
    }

    sushi::set_texture(0, tex_iter->second);
    sushi::draw_mesh(sprite_mesh);
}

void game_renderer::finish() {}

void game_renderer::update_3d_audio(const glm::vec2& pos) {
    soloud.set3dListenerPosition(pos.x, pos.y, 0.f);
    soloud.set3dListenerUp(0.f, 0.f, 1.f);
    soloud.update3dAudio();
}

void game_renderer::play_sfx(const std::string& name) {
    soloud.play(load_sfx(name));
}

void game_renderer::play_sfx(const std::string& name, const glm::vec2& pos) {
    soloud.play3d(load_sfx(name), pos.x, pos.y, 0.f);
}

void game_renderer::play_bgm(const std::string& name) {
    auto iter = bgm.find(name);
    if (iter == bgm.end()) {
        iter = bgm.emplace(name, SoLoud::WavStream{}).first;
        iter->second.load(("data/sounds/" + name).c_str());
        iter->second.setLooping(1);
    }

    if (bgm_handle) {
        soloud.stop(*bgm_handle);
    }

    bgm_handle = soloud.playBackground(iter->second);
    soloud.setVolume(*bgm_handle, 0.5);
}

auto game_renderer::load_sfx(const std::string& name) -> SoLoud::Wav& {
    auto iter = sfx.find(name);
    if (iter == sfx.end()) {
        iter = sfx.emplace(name, std::make_unique<SoLoud::Wav>()).first;
        iter->second->load(("data/sounds/" + name).c_str());
        iter->second->set3dMinMaxDistance(1.f, 10.f);
        iter->second->set3dAttenuation(SoLoud::AudioSource::INVERSE_DISTANCE, 11.f);
    }

    return *iter->second;
}
