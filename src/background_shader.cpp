#include "background_shader.hpp"

background_shader_program::background_shader_program(const std::string& vert, const std::string& frag) :
    sushi::shader_base({
        {sushi::shader_type::VERTEX, vert},
        {sushi::shader_type::FRAGMENT, frag},
    })
{
    bind();

    uniforms.MVP = get_uniform_location("MVP");
    uniforms.modelmat = get_uniform_location("ModelMat");
    uniforms.time = get_uniform_location("Time");
    uniforms.worldsize = get_uniform_location("WorldSize");
}

void background_shader_program::set_MVP(const glm::mat4& mat) {
    sushi::set_current_program_uniform(uniforms.MVP, mat);
}

void background_shader_program::set_modelmat(const glm::mat4& mat) {
    sushi::set_current_program_uniform(uniforms.modelmat, mat);
}

void background_shader_program::set_time(int time) {
    sushi::set_current_program_uniform(uniforms.time, time);
}

void background_shader_program::set_worldsize(const glm::vec2& worldsize) {
    sushi::set_current_program_uniform(uniforms.worldsize, worldsize);
}
