#include "UnlitRenderer.hpp"

#include <stdexcept>

#include "../Renderer.hpp"
namespace vkr {
struct GPUDrawPushConstants {
    glm::mat4 projViewMatrix;
    VkDeviceAddress vertexBuffer;
    VkDeviceAddress instanceBuffer;
};

void UnlitRenderer::initPipeline() {
    if (pipeline || pipelineLayout) {
        throw std::runtime_error("Cannot init UnlitRenderer pipeline twice");
    }
    auto device = render.getSystem().device;
    const auto& depthImage = render.getDepthImage();
    const auto& drawImage = render.getImage();

    VkShaderModule triangleFragShader =
        vk::loadShaderModule(RESPATH "/shaders/test.frag.spv", device);

    VkShaderModule triangleVertexShader =
        vk::loadShaderModule(RESPATH "/shaders/test.vert.spv", device);

    VkPushConstantRange bufferRange{};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo pipeline_layout_info =
        vk::pipelineLayoutCreateInfo();
    pipeline_layout_info.pPushConstantRanges = &bufferRange;
    pipeline_layout_info.pushConstantRangeCount = 1;

    vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr,
                           &pipelineLayout);

    vk::PipelineBuilder pipelineBuilder;

    pipelineBuilder._pipelineLayout = pipelineLayout;

    pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
    pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
    pipelineBuilder.setCullMode(VK_CULL_MODE_NONE,
                                VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pipelineBuilder.setMultisamplingNone();
    pipelineBuilder.disableBlending();
    pipelineBuilder.enableDepthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    pipelineBuilder.setDepthFormat(depthImage.imageFormat);
    pipelineBuilder.setColorAttachmentFormat(drawImage.imageFormat);

    pipeline = pipelineBuilder.buildPipeline(device);

    vkDestroyShaderModule(device, triangleFragShader, nullptr);
    vkDestroyShaderModule(device, triangleVertexShader, nullptr);
}
UnlitRenderer::UnlitRenderer(Renderer& render) : render(render) {}
UnlitRenderer::~UnlitRenderer() {
    auto vma = render.getSystem().allocator;

    if (!empty) {
        vmaDestroyBuffer(vma, commandsBuffer.buffer, commandsBuffer.allocation);
        vmaDestroyBuffer(vma, instancesBuffer.buffer,
                         instancesBuffer.allocation);
    }
    if (pipeline) {
        vkDestroyPipeline(render.getSystem().device, pipeline, nullptr);
    }
    if (pipelineLayout) {
        vkDestroyPipelineLayout(render.getSystem().device, pipelineLayout,
                                nullptr);
    }
}
void UnlitRenderer::draw(VkCommandBuffer cmd) {
    if (empty) {
        return;
    }
    auto screenSize = render.getScreenSize();

    VkRenderingAttachmentInfo colorAttachment = vk::attachmentInfo(
        render.getImage().imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingAttachmentInfo depthAttachment =
        vk::depthAttachmentInfo(render.getDepthImage().imageView,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo =
        vk::renderingInfo({.width = screenSize.w, .height = screenSize.h},
                          &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = (float)screenSize.w;
    viewport.height = (float)screenSize.h;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = screenSize.w;
    scissor.extent.height = screenSize.h;

    vkCmdSetScissor(cmd, 0, 1, &scissor);

    const auto& meshBuffer = render.meshHandler.getMesh();

    GPUDrawPushConstants push_constatns;
    push_constatns.projViewMatrix = glm::mat4(1);
    push_constatns.vertexBuffer = meshBuffer.vertexBufferAddr;
    push_constatns.instanceBuffer = instancesAddr;
    // push_constatns.instancesBuffer = instancesAddr;
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(GPUDrawPushConstants), &push_constatns);
    vkCmdBindIndexBuffer(cmd, meshBuffer.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexedIndirect(cmd, commandsBuffer.buffer, 0, commands.size(),
                             sizeof(VkDrawIndexedIndirectCommand));
    vkCmdEndRendering(cmd);
}
void UnlitRenderer::clear() {
    empty = true;
    commands.clear();
    instances.clear();
    render.destroyBuffer(commandsBuffer);
    render.destroyBuffer(instancesBuffer);
}
void UnlitRenderer::build() {
    if (!commands.size()) {
        return;
    }
    commandsBuffer = render.uploadBuffer(
        commands.data(), commands.size() * sizeof(VkDrawIndexedIndirectCommand),
        VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    instancesBuffer = render.uploadBuffer(
        instances.data(), instances.size() * sizeof(InstanceData),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = instancesBuffer.buffer};
    instancesAddr =
        vkGetBufferDeviceAddress(render.getSystem().device, &deviceAdressInfo);
    empty = false;
}
void UnlitRenderer::addInstance(MeshHandle* handle,
                                const glm::mat4& transform) {
    VkDrawIndexedIndirectCommand command;
    command.firstIndex = handle->firstIndex;
    command.firstInstance = commands.size();
    command.indexCount = handle->numIndex;
    command.instanceCount = 1;
    command.vertexOffset = handle->firstVertex;
    commands.push_back(command);

    InstanceData instance;
    instance.transform = transform;
    instances.push_back(instance);
}
}  // namespace vkr