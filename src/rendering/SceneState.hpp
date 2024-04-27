#pragma once

#include <unordered_set>

#include "../types.hpp"
#include "BufferWritter.hpp"
#include "GlobalBounds.hpp"
#include "StorageGPUVector.hpp"
#include "vk/vkApp.hpp"
#include "vk/vkDescriptors.hpp"

namespace vkr {
class BufferWritter;
class Renderer;

using BufferHandle = uint32_t;
using MaterialHandle = uint32_t;

using CullingType = uint32_t;
constexpr uint32_t CULLING_TYPE_FRUSTUM = 1;
constexpr uint32_t CULLING_TYPE_NONE = 0;
constexpr uint32_t CULLING_TYPE_OCCLUSION = 2;
constexpr uint32_t CULLING_TYPE_ALL = 3;

constexpr uint32_t MAX_DRAW_COMMANDS = 1024 * 1024 * 10;

struct TextureData {
    vk::AllocatedImage image;
    TextureBind bindPoint;
};

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
        glm::vec4 sphere;
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t vertexOffset;
        uint32_t material;
    };

    struct DrawParams {
        glm::mat4 transform;
        uint32_t materialId;
        uint32_t pad[3];
    };

    struct MaterialData {
        glm::vec4 color;
        TextureBind texColor;
        TextureBind texRoughMet;
        TextureBind texNormal;
        uint32_t pad;
    };

    using MaterialHandle = StorageGPUVector<MaterialData>::Handle;

   private:
    struct MeshAllocationData {
        uint32_t baseIndex{};
        uint32_t indicesCount{};
        uint32_t baseVertex{};
        glm::vec4 sphere;
    };

    static SceneState* instance;
    vk::vkApp& app;
    BufferWritter& bufferWritter;

    GlobalBounds bounds;

    StorageGPUVector<DrawCommand> drawCommands{0, bounds, MAX_DRAW_COMMANDS};
    StorageGPUVector<VkDrawIndexedIndirectCommand> cmdDraws{
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, bounds, MAX_DRAW_COMMANDS};
    StorageGPUVector<DrawCommandDataBuffer> drawCommandData{
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, bounds, 1};
    StorageGPUVector<DrawParams> drawParams{0, bounds, MAX_DRAW_COMMANDS};
    StorageGPUVector<MaterialData> materials{0, bounds};
    StorageGPUVector<Vertex> vertices{VK_BUFFER_USAGE_VERTEX_BUFFER_BIT};
    StorageGPUVector<uint32_t> indices{VK_BUFFER_USAGE_INDEX_BUFFER_BIT};

    std::vector<MeshAllocationData> meshesData;

    SceneState();

   public:
    ~SceneState();

    void update();

    void clearScene();

    BufferHandle allocateMesh(const MeshData& data);
    void clearMeshes();

    TextureData allocateTexture(void* data, VkExtent3D size, VkFormat format,
                                VkImageUsageFlags usage,
                                GlobalBounds::SamplerType sampler);
    void freeTexture(TextureData& data);

    void addInstance(BufferHandle mesh, const glm::mat4& transform,
                     MaterialHandle material) {
        auto& buffer = meshesData[mesh];

        glm::vec4 sphere = transform * glm::vec4(glm::vec3(buffer.sphere), 1);
        sphere.w = buffer.sphere.w *
                   glm::max(glm::max(transform[0][0], transform[1][1]),
                            transform[2][2]);

        DrawCommand dc{.transform = transform,
                       .sphere = sphere,
                       .firstIndex = buffer.baseIndex,
                       .indexCount = buffer.indicesCount,
                       .vertexOffset = (int32_t)buffer.baseVertex,
                       .material = material};
        drawCommands.add(dc);
    }

    StorageGPUVector<DrawCommand>& getDrawCommands() { return drawCommands; };
    StorageGPUVector<VkDrawIndexedIndirectCommand>& getCmdDraws() {
        return cmdDraws;
    }
    StorageGPUVector<DrawCommandDataBuffer>& getDrawCommandData() {
        return drawCommandData;
    }
    StorageGPUVector<DrawParams>& getDrawParams() { return drawParams; }
    StorageGPUVector<MaterialData>& getMaterials() { return materials; }
    StorageGPUVector<Vertex>& getVertices() { return vertices; }
    StorageGPUVector<uint32_t>& getIndices() { return indices; }

    GlobalBounds& getBounds() { return bounds; }

    static SceneState& get() { return *instance; }

    struct GlobalData {
        glm::mat4 proj, view;
        glm::vec3 camPosition;
        glm::vec3 clearColor;
        CullingType culling;
    } global;
};
}  // namespace vkr