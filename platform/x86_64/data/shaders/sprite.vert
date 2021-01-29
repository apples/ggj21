#version 400

in vec3 VertexPosition;
in vec2 VertexTexCoord;

uniform mat4 MVP;
uniform mat4 ModelMat;
uniform mat3 TexCoordMat;

out vec2 texcoord;
out vec3 worldCoord;

void main() {
    gl_Position = MVP * vec4(VertexPosition, 1.0);
    worldCoord = (ModelMat * vec4(VertexPosition, 1.0)).xyz;
    texcoord = vec2(TexCoordMat * vec3(VertexTexCoord, 1.0));
}
