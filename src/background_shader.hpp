#pragma once

#include <sushi/shader.hpp>
#include <glm/glm.hpp>

#include <string>

class background_shader_program : public sushi::shader_base {
public:
    background_shader_program() = default;

    background_shader_program(const std::string& vert, const std::string& frag);

    void set_MVP(const glm::mat4& mat);
    void set_uvmat(const glm::mat3& mat);
    void set_time(const int time);

private:
    struct {
        GLint MVP;
        GLint uvmat;
        GLint time;
    } uniforms;
};
