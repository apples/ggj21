#pragma once

#include <sushi/shader.hpp>
#include <glm/glm.hpp>

#include <string>

class sprite_shader_program : public sushi::shader_base {
public:
    sprite_shader_program() = default;

    sprite_shader_program(const std::string& vert, const std::string& frag);

    void set_MVP(const glm::mat4& mat);
    void set_uvmat(const glm::mat3& mat);
    void set_s_texture(GLint i);
    void set_tint(const glm::vec4& v);
    void set_invert(bool invert);

private:
    struct {
        GLint MVP;
        GLint uvmat;
        GLint s_texture;
        GLint tint;
        GLint invert;
    } uniforms;
};
