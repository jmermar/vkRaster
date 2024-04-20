#version 450

// output write
layout(location = 0) out vec4 outFragColor;

layout(binding = 2) uniform sampler2D textures[];

void main() {
    // return red
    outFragColor = texture(textures[1], vec2(1, 1));
}
