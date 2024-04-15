#pragma once
#include "../vk/vkApp.hpp"
#include "../vk/vkPipelines.hpp"

namespace vkr {
class SceneState;
class UnlitPass {
    friend class Renderer;

   private:
    vk::vkApp& app{vk::vkApp::get()};
    SceneState& sceneState;

    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline{};

    void buildPipeline();
    void buildPipelineLayout();

    UnlitPass();

   public:
    ~UnlitPass();

    void render(VkCommandBuffer cmd);
};
}  // namespace vkr