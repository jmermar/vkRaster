#include "CullingPass.hpp"

#include <glm/gtc/matrix_access.hpp>

#include "../SceneState.hpp"

namespace vkr {
struct GPUCullPushConstants {
    glm::vec4 left, right, front, back, top, bottom;
    StorageBind indirectDrawBind;
    StorageBind multiDrawDataBind;
    StorageBind instancesBind;
    StorageBind drawParamsBind;
};
void CullingPass::buildPipeline() {
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUCullPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo layoutCreate = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCreate.pushConstantRangeCount = 1;
    layoutCreate.pPushConstantRanges = &bufferRange;
    layoutCreate.pSetLayouts = &sceneState.getBounds().getLayout();
    layoutCreate.setLayoutCount = 1;

    vkCreatePipelineLayout(app.system.device, &layoutCreate, 0,
                           &pipelineLayout);

    VkComputePipelineCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

    VkPipelineShaderStageCreateInfo stageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

    stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stageInfo.pName = "main";
    stageInfo.module = vk::loadShaderModule(
        RESPATH "shaders/dispatcher.comp.spv", app.system.device);

    createInfo.layout = pipelineLayout;
    createInfo.stage = stageInfo;

    vkCreateComputePipelines(app.system.device, 0, 1, &createInfo, 0,
                             &pipeline);

    vkDestroyShaderModule(app.system.device, stageInfo.module, nullptr);
}
CullingPass::CullingPass() : sceneState(SceneState::get()) { buildPipeline(); }
CullingPass::~CullingPass() {
    vkDestroyPipeline(app.system.device, pipeline, nullptr);
    vkDestroyPipelineLayout(app.system.device, pipelineLayout, nullptr);
}
void CullingPass::render(VkCommandBuffer cmd) {
    VkMemoryBarrier2 memoryBarrier{.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
    memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    memoryBarrier.srcAccessMask =
        VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    memoryBarrier.dstAccessMask =
        VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
    VkDependencyInfo depInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
    depInfo.pMemoryBarriers = &memoryBarrier;
    depInfo.memoryBarrierCount = 1;

    vkCmdPipelineBarrier2(cmd, &depInfo);

    vkCmdFillBuffer(cmd, sceneState.getDrawCommandData().getBuffer(),
                    offsetof(SceneState::DrawCommandDataBuffer, drawCounts),
                    sizeof(uint32_t), 0);
    vkCmdFillBuffer(cmd, sceneState.getDrawCommandData().getBuffer(),
                    offsetof(SceneState::DrawCommandDataBuffer, maxDraws),
                    sizeof(uint32_t), sceneState.getDrawCommands().getSize());
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                            0, 1, &sceneState.getBounds().getDescriptor(), 0,
                            0);

    GPUCullPushConstants push;
    push.indirectDrawBind = sceneState.getCmdDraws().getBindPoint();
    push.multiDrawDataBind = sceneState.getDrawCommandData().getBindPoint();
    push.instancesBind = sceneState.getDrawCommands().getBindPoint();
    push.drawParamsBind = sceneState.getDrawParams().getBindPoint();

    auto m = sceneState.global.proj * sceneState.global.view;

    push.right.x = m[0][3] + m[0][0];
    push.right.y = m[1][3] + m[1][0];
    push.right.z = m[2][3] + m[2][0];
    push.right.w = m[3][3] + m[3][0];

    push.left.x = m[0][3] - m[0][0];
    push.left.y = m[1][3] - m[1][0];
    push.left.z = m[2][3] - m[2][0];
    push.left.w = m[3][3] - m[3][0];

    push.top.x = m[0][3] - m[0][1];
    push.top.y = m[1][3] - m[1][1];
    push.top.z = m[2][3] - m[2][1];
    push.top.w = m[3][3] - m[3][1];

    push.bottom.x = m[0][3] + m[0][1];
    push.bottom.y = m[1][3] + m[1][1];
    push.bottom.z = m[2][3] + m[2][1];
    push.bottom.w = m[3][3] + m[3][1];

    push.back.x = m[0][2];
    push.back.y = m[1][2];
    push.back.z = m[2][2];
    push.back.w = m[3][2];

    push.front.x = m[0][3] - m[0][2];
    push.front.y = m[1][3] - m[1][2];
    push.front.z = m[2][3] - m[2][2];
    push.front.w = m[3][3] - m[3][2];

    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                       sizeof(GPUCullPushConstants), &push);

    vkCmdDispatch(cmd, sceneState.getDrawCommands().getSize(), 1, 1);

    memoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    vkCmdPipelineBarrier2(cmd, &depInfo);
}
}  // namespace vkr