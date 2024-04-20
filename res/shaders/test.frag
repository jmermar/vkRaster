#version 450
#extension GL_EXT_nonuniform_qualifier : require

// output write
layout(location = 0) out vec4 outFragColor;

layout(location = 0) in vec3 color;
layout(location = 1) in vec2 uv;
layout(location = 2) in flat uint material;

layout(binding = 2) uniform sampler2D textures[];

struct Material {
    vec4 color;
    uint textureId;
    uint pad[3];
};

layout(binding = 1, std430) readonly buffer Materials { Material materials[]; }
materials[];

layout(push_constant) uniform constants {
    mat4 projView;
    uint drawParamsBind;
    uint materialsBind;
};

void main() {
    uint textureId = materials[materialsBind].materials[material].textureId;
    if (textureId > 0) {
        outFragColor = texture(textures[textureId], uv);
    } else {
        outFragColor = materials[materialsBind].materials[material].color;
    }
}
