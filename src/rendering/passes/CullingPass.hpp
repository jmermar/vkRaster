#pragma once
#include "../vk/vkApp.hpp"
#include "../vk/vkPipelines.hpp"

namespace vkr {
class SceneState;

class CullingPass {
    friend class Renderer;

   private:
    vk::vkApp& app{vk::vkApp::get()};
    SceneState& sceneState;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    void buildPipeline();

    CullingPass();

   public:
    ~CullingPass();

    void render(VkCommandBuffer cmd);
};
}  // namespace vkr