#include "Renderer.hpp"

#include <cmath>
#include <iostream>

#include "BufferWritter.hpp"
#include "SceneState.hpp"
#include "passes/CullingPass.hpp"
#include "passes/ImGUIPass.hpp"
#include "passes/UnlitPass.hpp"
#include "vk/vkApp.hpp"
#include "vk/vkCommand.hpp"
#include "vk/vkPipelines.hpp"
namespace vkr {
Renderer::Renderer(SDL_Window* window, uint32_t w, uint32_t h)
    : app(vk::vkApp::get()) {
    app.init(window, w, h);
    bufferWritter = new BufferWritter();
    sceneState = new SceneState();
    unlitPass = new UnlitPass();
    cullingPass = new CullingPass();
    imGUIPass = new ImGUIPass();
    screenSize = {w, h};
}

Renderer::~Renderer() {
    vkDeviceWaitIdle(app.system.device);
    delete cullingPass;
    delete unlitPass;
    delete sceneState;
    delete bufferWritter;
    app.finish();
}

void Renderer::render(glm::vec4 clearColor) {
    sceneState->update();

    const auto& system = app.system;
    auto device = system.device;
    vk::vkApp::FrameData* frameP;
    if (!app.renderBegin(&frameP)) {
        return;
    }
    auto& frame = *frameP;
    auto renderFence = frame.renderFence;
    auto renderSemaphore = frame.renderSemaphore;
    auto swapchainSemaphore = frame.swapchainSemaphore;
    auto buffer = frame.buffer;

    cullingPass->render(buffer);

    vkCommands::transitionImage(buffer, app.drawImage.image,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_GENERAL);

    VkClearColorValue clearValue = {
        {clearColor.r, clearColor.g, clearColor.b, 1.0f}};

    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    vkCmdClearColorImage(buffer, app.drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                         &clearValue, 1, &clearRange);

    vkCommands::transitionImage(buffer, app.drawImage.image,
                                VK_IMAGE_LAYOUT_GENERAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    unlitPass->render(buffer);

    imGUIPass->render(buffer);

    // Draw scene

    app.renderEnd();
}
}  // namespace vkr