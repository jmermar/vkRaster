#pragma once
#include "../SceneState.hpp"
#include "../vk/vkApp.hpp"
#include "../vk/vkPipelines.hpp"
namespace vkr {
class SceneState;
class ImGUIPass {
    friend class Renderer;

   private:
    vk::vkApp& app{vk::vkApp::get()};
    SceneState& sceneState;

    ImGUIPass();

   public:
    ~ImGUIPass();

    void render(VkCommandBuffer cmd);
};
}  // namespace vkr