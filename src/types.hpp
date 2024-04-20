#pragma once
#include <stdint.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>
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

struct ImageData {
    uint32_t width{}, height{};
    std::vector<uint32_t> data{};
};

struct TransformData {
    glm::vec3 position, rotation, scale;
    glm::mat4 getTransform();
};
struct CameraData {
    glm::vec3 position{0.f, 0.f, 0.f};
    glm::vec3 target{0.f, 0.f, 1.f};

    void rotateX(float degrees);
    void rotateY(float degrees);

    inline void lookAt(const glm::vec3& pos) {
        target = glm::normalize(pos - position);
    }

    float fov{45.f}, w{1}, h{1}, znear{0.1f}, zfar{10000.f};

    glm::mat4 getView();
    glm::mat4 getProj();
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct Material {
    glm::mat4 color;
    uint32_t texture;
};
}  // namespace vkr