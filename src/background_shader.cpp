#include "background_shader.hpp"

background_shader_program::background_shader_program(const std::string& vert, const std::string& frag) :
    sushi::shader_base({
        {sushi::shader_type::VERTEX, vert},
        {sushi::shader_type::FRAGMENT, frag},
    })
{
    bind();

    uniforms.MVP = get_uniform_location("MVP");
    uniforms.uvmat = get_uniform_location("TexCoordMat");
    uniforms.time = get_uniform_location("time");
}

void background_shader_program::set_MVP(const glm::mat4& mat) {
    sushi::set_current_program_uniform(uniforms.MVP, mat);
}

void background_shader_program::set_uvmat(const glm::mat3& mat) {
    sushi::set_current_program_uniform(uniforms.uvmat, mat);
}

void background_shader_program::set_time(const int time) {
    sushi::set_current_program_uniform(uniforms.time, time);
}