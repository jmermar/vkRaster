#include "ImGUIPass.hpp"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "../GlobalRenderData.hpp"
#include "../vk/vkCommand.hpp"

struct GPUPushConstants {
    vkr::TextureBind source;
};

namespace vkr {
ImGUIPass::ImGUIPass() : sceneState(SceneState::get()) {}
ImGUIPass::~ImGUIPass() {}
void ImGUIPass::render(VkCommandBuffer cmd) {
    auto size = app.getScreenSize();

    VkRenderingAttachmentInfo colorAttachment =
        vk::attachmentInfo(app.drawImage.imageView, nullptr,
                           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    VkRenderingInfo renderInfo = vk::renderingInfo(
        {.width = size.w, .height = size.h}, &colorAttachment, nullptr);

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);
}
}  // namespace vkr