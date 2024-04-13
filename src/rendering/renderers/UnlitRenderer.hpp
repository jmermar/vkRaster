#pragma once
#include "../BindlessDescriptor.hpp"
#include "../vk/vkBuffer.hpp"
#include "../vk/vkPipelines.hpp"
#include "rendering/MeshHandler.hpp"
namespace vkr {
class UnlitRenderer {
   private:
    struct InstanceData {
        glm::mat4 transform;
    };

    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline{};

    BufferBindHandle instancesBind;

    Renderer& render;
    std::vector<VkDrawIndexedIndirectCommand> commands;
    std::vector<InstanceData> instances;
    vk::AllocatedBuffer commandsBuffer;
    vk::AllocatedBuffer instancesBuffer;

    bool empty{true};

   public:
    UnlitRenderer(Renderer& render);
    ~UnlitRenderer();

    void initPipeline();

    void draw(VkCommandBuffer buffer);

    void clear();
    void build();

    void addInstance(MeshHandle* handle, const glm::mat4& transform);
};
}  // namespace vkr