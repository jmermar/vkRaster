#include "ImGUIPass.hpp"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "../vk/vkCommand.hpp"

struct GPUPushConstants {
    vkr::TextureBind source;
};

namespace vkr {
ImGUIPass::ImGUIPass() : sceneState(SceneState::get()) {}
ImGUIPass::~ImGUIPass() {}
void ImGUIPass::render(VkCommandBuffer cmd) {
    uint32_t w = app.getScreenW();
    uint32_t h = app.getScreenH();

    VkRenderingAttachmentInfo colorAttachment =
        vk::attachmentInfo(app.drawImage.imageView, nullptr,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo =
        vk::renderingInfo({.width = w, .height = h}, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}
}  // namespace vkr