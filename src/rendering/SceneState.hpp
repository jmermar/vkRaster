#pragma once

#include "../types.hpp"
#include "vk/vkApp.hpp"
#include "vk/vkDescriptors.hpp"

namespace vkr {
class BufferWritter;
using BufferHandle = uint32_t;
class Renderer;

enum StorageBind : uint32_t;
enum UniformBind : uint32_t;
enum TextureBind : uint32_t;
constexpr uint32_t MAX_DRAW_COMMANDS = 1024 * 1024;

struct StorageBufferDesc {
    StorageBind bind{};
    VkBuffer buffer{};
};

class SceneState {
    friend class Renderer;

   public:
    struct DrawCommandDataBuffer {
        uint32_t drawCounts;
        uint32_t maxDraws;
        uint32_t pad[2];
    };
    struct DrawCommand {
        glm::mat4 transform;
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t vertexOffset;
        uint32_t material;
    };

   private:
    struct MeshAllocationData {
        uint32_t baseIndex{};
        uint32_t indicesCount{};
        uint32_t baseVertex{};
    };

    static SceneState* instance;
    vk::vkApp& app;
    BufferWritter& bufferWritter;

    std::vector<DrawCommand> drawCommands;
    bool drawCommandsDirty{};
    vk::AllocatedBuffer drawCommandsBuffer{};
    vk::AllocatedBuffer cmdDrawsBuffer{};
    vk::AllocatedBuffer drawCommandDataBuffer{};
    StorageBufferDesc drawCommandsBufferDesc{};
    StorageBufferDesc cmdDrawsBufferDesc{};
    StorageBufferDesc drawCommandDataBufferDesc{};

    std::vector<MeshAllocationData> meshesData;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vk::AllocatedBuffer verticesBuffer{};
    vk::AllocatedBuffer indicesBuffer{};
    bool meshDirty{};

    vk::DescriptorAllocator descAlloc;
    VkDescriptorSetLayout descLayout;
    VkDescriptorSet descriptor;
    std::vector<bool> storageBounds;
    std::vector<bool> uniformBounds;

    void loadDescriptors();
    void loadDrawCommandsBuffers();

    SceneState();

   public:
    ~SceneState();

    void update();

    BufferHandle allocateMesh(const MeshData& data);
    void clearMeshes();

    void addInstance(BufferHandle mesh, const glm::mat4& transform);
    void clearInstances();

    UniformBind bindUniform(VkBuffer buffer);
    StorageBind bindStorage(VkBuffer buffer);

    void removeBind(UniformBind bind);
    void removeBind(StorageBind bind);

    inline const VkBuffer& getIndexBuffer() { return indicesBuffer.buffer; }
    inline const VkBuffer& getVertexBuffer() { return verticesBuffer.buffer; }

    inline size_t getNumberOfInstances() { return drawCommands.size(); }

    inline const VkDescriptorSetLayout& getDescriptorSetLayout() {
        return descLayout;
    }

    inline const VkDescriptorSet& getGlobalDescriptorSet() {
        return descriptor;
    }

    inline const StorageBufferDesc& getCmdDrawsBuffer() {
        return cmdDrawsBufferDesc;
    }
    inline const StorageBufferDesc& getDrawsCommandDataBuffer() {
        return drawCommandDataBufferDesc;
    }
    inline const StorageBufferDesc& getDrawsCommandsBuffer() {
        return drawCommandsBufferDesc;
    }

    static SceneState& get() { return *instance; }

    struct GlobalData {
        glm::mat4 proj, view;
        glm::vec3 clearColor;
    };
};
}  // namespace vkr