#include "LightCullingPass.hpp"

#include <glm/gtc/matrix_access.hpp>

#include "../GlobalRenderData.hpp"
#include "../SceneState.hpp"

namespace vkr {
struct GPUCullPushConstants {
    glm::mat4 view;
    glm::mat4 invP;
    glm::mat4 proj;

    int screenX;
    int screenY;

    int lightCount;

    StorageBind lightsBind;
    StorageBind lightIndexBind;
    TextureBind depthBind;
};
void LightCullingPass::buildPipeline() {
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
        RESPATH "shaders/lightCulling.comp.spv", app.system.device);

    createInfo.layout = pipelineLayout;
    createInfo.stage = stageInfo;

    vkCreateComputePipelines(app.system.device, 0, 1, &createInfo, 0,
                             &pipeline);

    vkDestroyShaderModule(app.system.device, stageInfo.module, nullptr);
}
LightCullingPass::LightCullingPass() : sceneState(SceneState::get()) {
    buildPipeline();
}
LightCullingPass::~LightCullingPass() {
    vkDestroyPipeline(app.system.device, pipeline, nullptr);
    vkDestroyPipelineLayout(app.system.device, pipelineLayout, nullptr);
}
void LightCullingPass::render(VkCommandBuffer cmd) {
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
    static TextureBind depthBind = (TextureBind)0;
    sceneState.getBounds().removeBind(depthBind);
    depthBind = sceneState.getBounds().bindTexture(
        app.depthImage.imageView, GlobalBounds::SAMPLER_NEAREST);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
                            0, 1, &sceneState.getBounds().getDescriptor(), 0,
                            0);
    auto& global = GlobalRenderData::get();
    GPUCullPushConstants push;
    push.view = global.viewMatrix;
    push.invP = glm::inverse(global.projMatrix);
    push.proj = global.projMatrix;
    push.screenX = global.windowSize.w;
    push.screenY = global.windowSize.h;
    push.lightCount = sceneState.getLights().getLogicalSize();
    push.lightIndexBind = sceneState.getLighTile().getBindPoint();
    push.lightsBind = sceneState.getLights().getBindPoint();

    push.depthBind = depthBind;

    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0,
                       sizeof(GPUCullPushConstants), &push);

    vkCmdDispatch(cmd, global.screenTileSize.w, global.screenTileSize.h, 1);

    memoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
    vkCmdPipelineBarrier2(cmd, &depInfo);
}
}  // namespace vkr