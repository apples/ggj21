#version 400

in vec2 texcoord;

uniform sampler2D DiffuseTex;
uniform vec4 Tint;
uniform bool Invert;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(DiffuseTex, texcoord);
    if (outColor.a < 0.5) discard;
    outColor = outColor * Tint;

    if (Invert) {
        outColor = vec4(vec3(1.0) - outColor.rgb, outColor.a);
    }
}
