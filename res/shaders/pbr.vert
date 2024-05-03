#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 vColor;
layout(location = 4) in vec3 vTangent;
layout(location = 5) in vec3 vBitangent;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outUV;
layout(location = 3) out vec3 outWorldPos;
layout(location = 4) out flat uint material;
layout(location = 5) out mat3 TBN;

// push constants block
layout(push_constant) uniform constants {
    mat4 projView;
    vec4 cameraPosition;
    uint lightsBind;
    uint drawParamsBind;
    uint materialsBind;
}
PushConstants;

struct DrawParam {
    mat4 transform;
    uint material;
    uint pad[3];
};
layout(binding = 1, std430) readonly buffer DrawParams { DrawParam params[]; }
drawParams[];

void main() {
    DrawParam drawParam =
        drawParams[PushConstants.drawParamsBind].params[gl_InstanceIndex];

    material = drawParam.material;
    outWorldPos = (drawParam.transform * vec4(vPosition, 1.0f)).xyz;

    vec3 T = normalize(vec3(drawParam.transform * vec4(vTangent, 0)));
    vec3 B = normalize(vec3(drawParam.transform * vec4(vBitangent, 0)));
    vec3 N = normalize(vec3(drawParam.transform * vec4(vNormal, 0)));

    TBN = mat3(T, B, N);

    gl_Position =
        PushConstants.projView * drawParam.transform * vec4(vPosition, 1.0f);
    outColor = vColor;
    outUV = vUV;
}
