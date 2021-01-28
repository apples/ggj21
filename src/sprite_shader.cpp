#include "sprite_shader.hpp"

sprite_shader_program::sprite_shader_program(const std::string& vert, const std::string& frag) :
    sushi::shader_base({
        {sushi::shader_type::VERTEX, vert},
        {sushi::shader_type::FRAGMENT, frag},
    })
{
    bind();

    uniforms.MVP = get_uniform_location("MVP");
    uniforms.uvmat = get_uniform_location("TexCoordMat");
    uniforms.s_texture = get_uniform_location("DiffuseTex");
    uniforms.tint = get_uniform_location("Tint");
}

void sprite_shader_program::set_MVP(const glm::mat4& mat) {
    sushi::set_current_program_uniform(uniforms.MVP, mat);
}

void sprite_shader_program::set_uvmat(const glm::mat3& mat) {
    sushi::set_current_program_uniform(uniforms.uvmat, mat);
}

void sprite_shader_program::set_s_texture(GLint i) {
    sushi::set_current_program_uniform(uniforms.s_texture, i);
}

void sprite_shader_program::set_tint(const glm::vec4& v) {
    sushi::set_current_program_uniform(uniforms.tint, v);
}
