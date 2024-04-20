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
SceneState::~SceneState() {
    for (auto t : textures) {
        app.deletion.addImage(t->image);
        delete t;
    }
}
void SceneState::update() {
    drawCommands.update();
    drawCommandData.update();
    cmdDraws.update();
    drawCommandData.update();
    drawParams.update();
    materials.update();
    vertices.update();
    indices.update();
}
void SceneState::clearScene() {
    clearMeshes();
    bounds.clearBounds();

    drawCommands.clear();
    drawCommandData.clear();
    cmdDraws.clear();
    drawCommandData.clear();
    drawParams.clear();
    materials.clear();
    vertices.clear();
    indices.clear();

    for (auto t : textures) {
        app.deletion.addImage(t->image);
        delete t;
    }
    textures.clear();
}
BufferHandle SceneState::allocateMesh(const MeshData& data) {
    uint32_t baseVertex = vertices.getSize();
    uint32_t baseIndex = indices.getSize();

    meshesData.push_back({.baseIndex = baseIndex,
                          .indicesCount = (uint32_t)data.indices.size(),
                          .baseVertex = baseVertex});
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

TextureData* SceneState::allocateTexture(
    void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage,
    GlobalBounds::SamplerType samplerType) {
    auto ret = new TextureData;
    auto device = app.system.device;
    auto allocator = app.system.allocator;
    vk::allocateImage(
        ret->image, device, allocator, size, format,
        usage | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    bufferWritter.writeImage(data, ret->image.image, size);

    ret->bindPoint = bounds.bindTexture(ret->image.imageView, samplerType);
    textures.insert(ret);
    return ret;
}

void SceneState::freeTexture(TextureData* data) {
    bounds.removeBind(data->bindPoint);
    app.deletion.addImage(data->image);
    textures.erase(data);
    delete data;
}
}  // namespace vkr