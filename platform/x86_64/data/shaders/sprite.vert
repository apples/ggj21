#version 400

in vec3 VertexPosition;
in vec2 VertexTexCoord;

uniform mat4 MVP;
uniform mat3 TexCoordMat;

out vec2 texcoord;

void main() {
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    texcoord = vec2(TexCoordMat * vec3(VertexTexCoord, 1.0));
}
