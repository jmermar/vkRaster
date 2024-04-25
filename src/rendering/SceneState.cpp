#include "SceneState.hpp"

#include <span>
#include <stdexcept>

#include "BufferWritter.hpp"
#include "vk/vkBuffer.hpp"
#include "vk/vkImage.hpp"
#define DESTROY_BUFFER(b)                                                   \
    {                                                                       \
        if (b.buffer) {                                                     \
            vmaDestroyBuffer(app.system.allocator, b.buffer, b.allocation); \
        }                                                                   \
    }

namespace vkr {
SceneState* SceneState::instance = 0;

SceneState::SceneState()
    : app(vk::vkApp::get()), bufferWritter(BufferWritter::get()) {
    instance = this;
}
SceneState::~SceneState() {}
void SceneState::update() {
    drawCommands.update();
    drawCommandData.update();
    cmdDraws.update();
    drawParams.update();
    materials.update();
    vertices.update();
    indices.update();
}
void SceneState::clearScene() {
    clearMeshes();

    drawCommands.clear();
    materials.clear();
}
BufferHandle SceneState::allocateMesh(const MeshData& data) {
    glm::vec3 minCorner = data.vertices[0].position,
              maxCorner = data.vertices[0].position;
    for (auto& v : data.vertices) {
        minCorner.x = std::min(minCorner.x, v.position.x);
        minCorner.y = std::min(minCorner.y, v.position.y);
        minCorner.z = std::min(minCorner.z, v.position.z);

        maxCorner.x = std::max(maxCorner.x, v.position.x);
        maxCorner.y = std::max(maxCorner.y, v.position.y);
        maxCorner.z = std::max(maxCorner.z, v.position.z);
    }

    glm::vec3 center = minCorner + maxCorner * 0.5f;
    float size = 0;
    for (auto& v : data.vertices) {
        size = std::max(glm::length(v.position - center), size);
    }

    uint32_t baseVertex = vertices.getLogicalSize();
    uint32_t baseIndex = indices.getLogicalSize();

    meshesData.push_back({.baseIndex = baseIndex,
                          .indicesCount = (uint32_t)data.indices.size(),
                          .baseVertex = baseVertex,
                          .sphere = glm::vec4(center, size)});
    std::span<const Vertex> verticesSpan(data.vertices);
    std::span<const uint32_t> indicesSpan(data.indices);
    vertices.insert(verticesSpan);
    indices.insert(indicesSpan);
    return meshesData.size() - 1;
}
void SceneState::clearMeshes() {
    meshesData.clear();
    vertices.clear();
    indices.clear();
}

TextureData SceneState::allocateTexture(void* data, VkExtent3D size,
                                        VkFormat format,
                                        VkImageUsageFlags usage,
                                        GlobalBounds::SamplerType samplerType) {
    TextureData ret{};
    auto device = app.system.device;
    auto allocator = app.system.allocator;
    vk::allocateImage(
        ret.image, device, allocator, size, format,
        usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    bufferWritter.writeImage(data, ret.image.image, size);

    ret.bindPoint = bounds.bindTexture(ret.image.imageView, samplerType);

    return ret;
}

void SceneState::freeTexture(TextureData& data) {
    bounds.removeBind(data.bindPoint);
    app.deletion.addImage(data.image);
}
}  // namespace vkr