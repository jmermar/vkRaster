#include "UnlitPass.hpp"

#include "../SceneState.hpp"

struct GPUDrawPushConstants {
    glm::mat4 projViewMatrix;
    vkr::StorageBind drawParamsBind;
    vkr::StorageBind materialsBind;
};

namespace vkr {
void UnlitPass::buildPipeline() {
    auto device = app.system.device;
    const auto& depthImage = app.depthImage;
    const auto& drawImage = app.drawImage;

    VkShaderModule triangleFragShader =
        vk::loadShaderModule(RESPATH "/shaders/test.frag.spv", device);

    VkShaderModule triangleVertexShader =
        vk::loadShaderModule(RESPATH "/shaders/test.vert.spv", device);

    vk::PipelineBuilder pipelineBuilder;

    pipelineBuilder._pipelineLayout = pipelineLayout;

    pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
    pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.setCullMode(VK_CULL_MODE_FRONT_BIT,
                                VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipelineBuilder.setMultisamplingNone();
    pipelineBuilder.disableBlending();
    pipelineBuilder.enableDepthtest(true, VK_COMPARE_OP_LESS_OR_EQUAL);
    pipelineBuilder.setDepthFormat(depthImage.imageFormat);
    pipelineBuilder.setColorAttachmentFormat(drawImage.imageFormat);

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    pipelineBuilder.vertexInputBindings.push_back(binding);

    VkVertexInputAttributeDescription desc{};
    desc.binding = 0;

    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(Vertex, position);
    desc.location = 0;
    pipelineBuilder.vertexInputAttributes.push_back(desc);

    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(Vertex, normal);
    desc.location = 1;
    pipelineBuilder.vertexInputAttributes.push_back(desc);

    desc.format = VK_FORMAT_R32G32_SFLOAT;
    desc.offset = offsetof(Vertex, uv);
    desc.location = 2;
    pipelineBuilder.vertexInputAttributes.push_back(desc);

    desc.format = VK_FORMAT_R32G32B32_SFLOAT;
    desc.offset = offsetof(Vertex, color);
    desc.location = 3;
    pipelineBuilder.vertexInputAttributes.push_back(desc);

    pipeline = pipelineBuilder.buildPipeline(device);

    vkDestroyShaderModule(device, triangleFragShader, nullptr);
    vkDestroyShaderModule(device, triangleVertexShader, nullptr);
}

void UnlitPass::buildPipelineLayout() {
    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info =
        vk::pipelineLayoutCreateInfo();
    pipeline_layout_info.pPushConstantRanges = &bufferRange;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = &sceneState.getBounds().getLayout();
    pipeline_layout_info.setLayoutCount = 1;

    vkCreatePipelineLayout(app.system.device, &pipeline_layout_info, nullptr,
                           &pipelineLayout);
}
UnlitPass::UnlitPass() : sceneState(SceneState::get()) {
    buildPipelineLayout();
    buildPipeline();
}
UnlitPass::~UnlitPass() {
    if (pipeline) {
        vkDestroyPipeline(app.system.device, pipeline, nullptr);
    }
    if (pipelineLayout) {
        vkDestroyPipelineLayout(app.system.device, pipelineLayout, nullptr);
    }
}
void UnlitPass::render(VkCommandBuffer cmd) {
    auto indexBuffer = sceneState.getIndices().getBuffer();
    if (!indexBuffer) {
        return;
    }
    uint32_t w = app.getScreenW();
    uint32_t h = app.getScreenH();

    VkRenderingAttachmentInfo colorAttachment = vk::attachmentInfo(
        app.drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingAttachmentInfo depthAttachment = vk::depthAttachmentInfo(
        app.depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo = vk::renderingInfo(
        {.width = w, .height = h}, &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, 0, 1,
                            &sceneState.getBounds().getDescriptor(), 0, 0);
    // set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)w;
    viewport.height = (float)h;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = w;
    scissor.extent.height = h;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    GPUDrawPushConstants push_constatns;
    push_constatns.projViewMatrix =
        sceneState.global.proj * sceneState.global.view;
    push_constatns.drawParamsBind = sceneState.getDrawParams().getBindPoint();
    push_constatns.materialsBind = sceneState.getMaterials().getBindPoint();

    vkCmdPushConstants(
        cmd, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(GPUDrawPushConstants), &push_constatns);
    vkCmdBindIndexBuffer(cmd, sceneState.getIndices().getBuffer(), 0,
                         VK_INDEX_TYPE_UINT32);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &sceneState.getVertices().getBuffer(),
                           &offset);

    vkCmdDrawIndexedIndirectCount(cmd, sceneState.getCmdDraws().getBuffer(), 0,
                                  sceneState.getDrawCommandData().getBuffer(),
                                  0, MAX_DRAW_COMMANDS,
                                  sizeof(VkDrawIndexedIndirectCommand));
    vkCmdEndRendering(cmd);
}
}  // namespace vkr