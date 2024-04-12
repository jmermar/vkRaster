#pragma once
#include <stdint.h>
#include <vector>

#include <glm/glm.hpp>
namespace vkr {
struct Size {
    uint32_t w, h;
};

struct Rect {
    int32_t x, y, w, h;
};

struct URect {
    uint32_t x, y, w, h;
};

struct IPoint {
    int32_t x, y;
};

struct Vertex {
    glm::vec3 position;
    float pad;
    glm::vec3 normal;
    float pad1;
    glm::vec2 uv;
    float pad2[2];
    glm::vec4 color;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
}  // namespace vkr