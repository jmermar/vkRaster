#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 vColor;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUV;

// push constants block
layout(push_constant) uniform constants {
    mat4 projView;
    uint drawParamsBind;
}
PushConstants;

struct DrawParam {
    mat4 transform;
    int pad[4];
};
layout(binding = 1, std430) readonly buffer DrawParams { DrawParam params[]; }
drawParams[];

void main() {
    DrawParam drawParam =
        drawParams[PushConstants.drawParamsBind].params[gl_InstanceIndex];
    gl_Position =
        PushConstants.projView * drawParam.transform * vec4(vPosition, 1.0f);
    outColor = vColor;
    outUV = vUV;
}
