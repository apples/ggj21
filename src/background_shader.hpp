#pragma once

#include <sushi/shader.hpp>
#include <glm/glm.hpp>

#include <string>

class background_shader_program : public sushi::shader_base {
public:
    background_shader_program() = default;

    background_shader_program(const std::string& vert, const std::string& frag);

    void set_MVP(const glm::mat4& mat);
    void set_modelmat(const glm::mat4& mat);
    void set_time(int time);
    void set_worldsize(const glm::vec2& worldsize);

private:
    struct {
        GLint MVP;
        GLint modelmat;
        GLint time;
        GLint worldsize;
    } uniforms;
};
