#pragma once
#include <vulkan/vulkan.h>

#include <types.hpp>
#include <vector>

#include "vk/vkBuffer.hpp"
#include "vk/vkInit.hpp"

namespace vkr {
struct GPUMesh {
    vk::AllocatedBuffer indexBuffer{};
    vk::AllocatedBuffer vertexBuffer{};
    size_t nVertices{}, nIndices{};
};

struct MeshHandle {
    uint32_t firstVertex;
    uint32_t firstIndex;
    uint32_t numVertex;
    uint32_t numIndex;
};

class Renderer;

class MeshHandler {
   private:
    Renderer& render;
    MeshData data;

    GPUMesh buffer;

   public:
    MeshHandler(Renderer& render);
    ~MeshHandler();

    MeshHandle* allocateMesh(const MeshData& data);

    void clear();

    void build();

    std::vector<MeshHandle> meshes;

    inline const GPUMesh& getMesh() { return buffer; }
};
}  // namespace vkr