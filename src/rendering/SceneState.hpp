#pragma once

#include "../types.hpp"
#include "vk/vkApp.hpp"

namespace vkr {
class BufferWritter;
using BufferHandle = uint32_t;
class Renderer;

class SceneState {
    friend class Renderer;

   private:
    struct DrawCommand {
        uint32_t baseVertex{};
        uint32_t baseIndex{};
        uint32_t indicesCount{};
        uint32_t textureHandle{};

        glm::mat4 transform;
    };

    struct MeshAllocationData {
        uint32_t baseIndex{};
        uint32_t indicesCount{};
        uint32_t baseVertex{};
    };

    static SceneState* instance;
    vk::vkApp& app;
    BufferWritter& bufferWritter;

    std::vector<DrawCommand> drawCommands;
    bool drawCommandDirty{};
    vk::AllocatedBuffer drawCommandsBuffer{};

    std::vector<MeshAllocationData> meshesData;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vk::AllocatedBuffer verticesBuffer{};
    vk::AllocatedBuffer indicesBuffer{};
    bool meshDirty{};

    SceneState();

   public:
    ~SceneState();

    void update();

    BufferHandle allocateMesh(const MeshData& data);
    void clearMeshes();

    static SceneState& get() { return *instance; }
};
}  // namespace vkr