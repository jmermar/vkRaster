#include "SceneState.hpp"

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
    DESTROY_BUFFER(drawCommandsBuffer);
    DESTROY_BUFFER(verticesBuffer);
    DESTROY_BUFFER(indicesBuffer);
}
void SceneState::update() {
    if (meshDirty) {
        meshDirty = false;
        app.deletion.addBuffer(verticesBuffer);
        app.deletion.addBuffer(indicesBuffer);

        if (indices.size() > 0) {
            uint32_t size = sizeof(uint32_t) * indices.size();
            indicesBuffer =
                vk::createBuffer(app.system.allocator, size,
                                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VMA_MEMORY_USAGE_GPU_ONLY);
            bufferWritter.writeBuffer(indices.data(), size,
                                      indicesBuffer.buffer);
        }
        if (vertices.size() > 0) {
            uint32_t size = sizeof(Vertex) * vertices.size();
            verticesBuffer =
                vk::createBuffer(app.system.allocator, size,
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                 VMA_MEMORY_USAGE_GPU_ONLY);
            bufferWritter.writeBuffer(vertices.data(), size,
                                      verticesBuffer.buffer);
        }
    }
}
BufferHandle SceneState::allocateMesh(const MeshData& data) {
    meshDirty = true;
    uint32_t baseVertex = vertices.size();
    uint32_t baseIndex = indices.size();

    meshesData.push_back({.baseIndex = baseIndex,
                          .indicesCount = (uint32_t)data.indices.size(),
                          .baseVertex = baseVertex});

    vertices.insert(vertices.end(), data.vertices.begin(), data.vertices.end());
    indices.insert(indices.end(), data.indices.begin(), data.indices.end());
    return meshesData.size() - 1;
}
void SceneState::clearMeshes() {
    meshesData.clear();
    vertices.clear();
    indices.clear();
    meshDirty = true;
}
}  // namespace vkr