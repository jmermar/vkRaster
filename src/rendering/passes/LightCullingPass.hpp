#pragma once
#include "../vk/vkApp.hpp"
#include "../vk/vkPipelines.hpp"

namespace vkr {
class SceneState;

class LightCullingPass {
    friend class Renderer;

   private:
    vk::vkApp& app{vk::vkApp::get()};
    SceneState& sceneState;

    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;

    void buildPipeline();

    LightCullingPass();

   public:
    ~LightCullingPass();

    void render(VkCommandBuffer cmd);
};
}  // namespace vkr