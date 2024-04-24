#version 450

layout(location = 0) out vec2 uv;

void main() {
    vec3 positions[6] = {
        vec3(-1, 1, 0), vec3(1, 1, 0),  vec3(1, -1, 0),

        vec3(-1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0),
    };

    gl_Position = vec4(positions[gl_VertexIndex], 1);
    uv = vec2(0.5) + positions[gl_VertexIndex].xy * 0.5;
}
