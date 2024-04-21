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
    uint32_t width{}, height{}, channels{4};
    std::vector<uint8_t> data{};
};

struct TransformData {
    glm::vec3 position{0.f, 0.f, 0.f}, rotation{0.f, 0.f, 0.f},
        scale{1.f, 1.f, 1.f};
    glm::mat4 getTransform() const;
};
struct CameraData {
    glm::vec3 position{0.f, 0.f, 0.f};
    glm::vec3 target{0.f, 0.f, 1.f};

    void rotateX(float degrees);
    void rotateY(float degrees);

    inline void lookAt(const glm::vec3& pos) {
        target = glm::normalize(pos - position);
    }

    float fov{45.f}, w{1}, h{1}, znear{0.1f}, zfar{1000.f};

    glm::mat4 getView();
    glm::mat4 getProj();
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct MaterialData {
    glm::vec4 color;
    int32_t texture;
};
}  // namespace vkr