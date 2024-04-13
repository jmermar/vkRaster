#pragma once
#include <stdint.h>

#include <glm/glm.hpp>
#include <vector>
#include <string>
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
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 color;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct MaterialData {
    std::string texture;
    glm::vec4 color;
};

struct ModelData {
    struct MeshPair {
        size_t materialId;
        MeshData mesh;
    };
    std::vector<MaterialData> materials;
    std::vector<MeshPair> meshes;
};
}  // namespace vkr