#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outFragColor;

layout(binding = 2) uniform sampler2D textures[];

layout(push_constant) uniform constants { uint textureID; };

void main() {
    vec4 color = texture(textures[textureID], uv);
    outFragColor = color;
}
