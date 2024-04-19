#include "SceneState.hpp"

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

const uint32_t UNIFORM_BIND = 0;
const uint32_t STORAGE_BIND = 1;
const uint32_t IMAGE_SAMPLER_BIND = 2;

size_t getFirstFree(std::vector<bool>& v) {
    int i = 0;
    for (auto b : v) {
        if (!b) {
            v[i] = true;
            return i;
        }
        i++;
    }
    v.push_back(true);
    return v.size();
}

namespace vkr {
SceneState* SceneState::instance = 0;

void SceneState::loadDescriptors() {
    vk::DescriptorLayoutBuilder layoutBuilder;
    auto& limits = app.system.physicalDeviceProperties.limits;
    layoutBuilder.addBinding(UNIFORM_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             limits.maxDescriptorSetUniformBuffers,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(STORAGE_BIND, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             limits.maxDescriptorSetStorageBuffers,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(IMAGE_SAMPLER_BIND,
                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             limits.maxDescriptorSetSampledImages,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

    descLayout = layoutBuilder.build(
        app.system.device,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        VK_SHADER_STAGE_ALL);

    vk::DescriptorAllocator::PoolSize sizes[3] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024}};

    descAlloc.initPool(app.system.device, sizes,
                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    descriptor = descAlloc.allocate(app.system.device, descLayout);
}

void SceneState::loadDrawCommandsBuffers() {
    const auto& vma = app.system.allocator;
    const auto& device = app.system.device;
    cmdDrawsBuffer = vk::createBuffer(
        vma, sizeof(VkDrawIndexedIndirectCommand) * MAX_DRAW_COMMANDS,
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    cmdDrawsBufferDesc.buffer = cmdDrawsBuffer.buffer;
    cmdDrawsBufferDesc.bind = bindStorage(cmdDrawsBuffer.buffer);

    drawCommandDataBuffer = vk::createBuffer(
        vma, sizeof(DrawCommandDataBuffer),
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    drawCommandDataBufferDesc.buffer = drawCommandDataBuffer.buffer;
    drawCommandDataBufferDesc.bind = bindStorage(drawCommandDataBuffer.buffer);

    drawParamsBuffer = vk::createBuffer(
        vma, sizeof(DrawParams) * MAX_DRAW_COMMANDS,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    drawParamsBufferDesc.buffer = drawParamsBuffer.buffer;
    drawParamsBufferDesc.bind = bindStorage(drawParamsBuffer.buffer);
}

SceneState::SceneState()
    : app(vk::vkApp::get()), bufferWritter(BufferWritter::get()) {
    instance = this;
    loadDescriptors();
    loadDrawCommandsBuffers();
}
SceneState::~SceneState() {
    DESTROY_BUFFER(cmdDrawsBuffer);
    DESTROY_BUFFER(drawCommandDataBuffer);
    DESTROY_BUFFER(drawCommandsBuffer);

    DESTROY_BUFFER(verticesBuffer);
    DESTROY_BUFFER(indicesBuffer);

    descAlloc.destroyPool(app.system.device);
    vkDestroyDescriptorSetLayout(app.system.device, descLayout, 0);
}
void SceneState::update() {
    if (drawCommandsDirty) {
        drawCommandsDirty = false;
        app.deletion.addBuffer(drawCommandsBuffer);
        removeBind(drawCommandsBufferDesc.bind);
        drawCommandsBufferDesc.bind = {};

        if (drawCommands.size() > 0) {
            uint32_t size = sizeof(DrawCommand) * drawCommands.size();
            drawCommandsBuffer =
                vk::createBuffer(app.system.allocator, size,
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VMA_MEMORY_USAGE_GPU_ONLY);
            bufferWritter.writeBuffer(drawCommands.data(), size,
                                      drawCommandsBuffer.buffer);
            drawCommandsBufferDesc.buffer = drawCommandsBuffer.buffer;
            drawCommandsBufferDesc.bind =
                bindStorage(drawCommandsBuffer.buffer);
        }
    }
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
void SceneState::addInstance(BufferHandle mesh, const glm::mat4& transform) {
    if (drawCommands.size() >= MAX_DRAW_COMMANDS) {
        throw std::runtime_error("Cannot add more instances");
    }
    const auto& meshData = meshesData[mesh];
    drawCommands.push_back({.transform = transform,
                            .firstIndex = meshData.baseIndex,
                            .indexCount = meshData.indicesCount,
                            .vertexOffset = (int32_t)meshData.baseVertex,
                            .material = 0});
    drawCommandsDirty = true;
}
void SceneState::clearInstances() {
    drawCommands.clear();
    drawCommandsDirty = true;
}
StorageBind SceneState::bindStorage(VkBuffer buffer) {
    size_t newHandle = getFirstFree(storageBounds);

    VkWriteDescriptorSet write = {.sType =
                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    write.dstSet = descriptor;
    write.dstBinding = STORAGE_BIND;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.dstArrayElement = newHandle;
    write.pBufferInfo = &bufferInfo;

    write.dstBinding = 1;

    vkUpdateDescriptorSets(app.system.device, 1, &write, 0, nullptr);

    return (StorageBind)newHandle;
}
UniformBind SceneState::bindUniform(VkBuffer buffer) {
    size_t newHandle = getFirstFree(uniformBounds);

    VkWriteDescriptorSet write = {.sType =
                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    write.dstSet = descriptor;
    write.dstBinding = UNIFORM_BIND;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.dstArrayElement = newHandle;
    write.pBufferInfo = &bufferInfo;

    write.dstBinding = 1;

    vkUpdateDescriptorSets(app.system.device, 1, &write, 0, nullptr);

    return (UniformBind)newHandle;
}
void SceneState::removeBind(UniformBind bind) {
    if (bind > 0) uniformBounds[bind - 1] = false;
}
void SceneState::removeBind(StorageBind bind) {
    if (bind > 0) storageBounds[bind - 1] = false;
}
}  // namespace vkr