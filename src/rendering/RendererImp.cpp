#include "RendererImp.hpp"

#include <cmath>
#include <iostream>

#include "vk/vkCommand.hpp"
#include "vk/vkPipelines.hpp"

namespace vkr {
VkSemaphoreSubmitInfo semaphore_submit_info(VkPipelineStageFlags2 stageMask,
                                            VkSemaphore semaphore) {
    VkSemaphoreSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.semaphore = semaphore;
    submitInfo.stageMask = stageMask;
    submitInfo.deviceIndex = 0;
    submitInfo.value = 1;

    return submitInfo;
}

VkCommandBufferSubmitInfo command_buffer_submit_info(VkCommandBuffer cmd) {
    VkCommandBufferSubmitInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    info.pNext = nullptr;
    info.commandBuffer = cmd;
    info.deviceMask = 0;

    return info;
}

VkSubmitInfo2 submit_info(VkCommandBufferSubmitInfo* cmd,
                          VkSemaphoreSubmitInfo* signalSemaphoreInfo,
                          VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;

    info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
    info.pWaitSemaphoreInfos = waitSemaphoreInfo;

    info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
    info.pSignalSemaphoreInfos = signalSemaphoreInfo;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = cmd;

    return info;
}

RendererImp::RendererImp(SDL_Window* window, uint32_t w, uint32_t h)
    : win(window), screenSize({w, h}) {
    auto device = system.get().device;
    vk::allocateImage(
        drawImage, device, system.get().allocator,
        {.width = w, .height = h, .depth = 1}, VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    vk::allocateImage(depthImage, device, system.get().allocator,
                      {.width = w, .height = h, .depth = 1},
                      VK_FORMAT_D32_SFLOAT,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_DEPTH_BIT);

    initDescriptorSets();
    initRenderPipeline();
}

void RendererImp::initRenderPipeline() {
    auto device = system.get().device;

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
                           &trianglePipelineLayout);

    vk::PipelineBuilder pipelineBuilder;

    // use the triangle layout we created
    pipelineBuilder._pipelineLayout = trianglePipelineLayout;
    // connecting the vertex and pixel shaders to the pipeline
    pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
    // it will draw triangles
    pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    // filled triangles
    pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
    // no backface culling
    pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
    // no multisampling
    pipelineBuilder.setMultisamplingNone();
    // no blending
    pipelineBuilder.disableBlending();
    // no depth testing
    pipelineBuilder.enableDepthtest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
    pipelineBuilder.setDepthFormat(depthImage.imageFormat);

    // connect the image format we will draw into, from draw image
    pipelineBuilder.setColorAttachmentFormat(drawImage.imageFormat);
    pipelineBuilder.setDepthFormat(depthImage.imageFormat);

    // finally build the pipeline
    trianglePipeline = pipelineBuilder.buildPipeline(device);

    // clean structures
    vkDestroyShaderModule(device, triangleFragShader, nullptr);
    vkDestroyShaderModule(device, triangleVertexShader, nullptr);
}

void RendererImp::initSubmit() {}

GPUMesh* RendererImp::uploadMesh(const std::string& name,
                                 const std::vector<uint32_t>& indices,
                                 const std::vector<Vertex>& vertices) {
    if (meshes.contains(name)) {
        throw std::runtime_error("Already a mesh with that name");
    }

    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);
    const auto allocator = system.get().allocator;
    const auto device = system.get().device;
    GPUMesh newSurface;

    // create vertex buffer
    newSurface.vertexBuffer = vk::createBuffer(
        allocator, vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    // find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer};
    newSurface.vertexBufferAddr =
        vkGetBufferDeviceAddress(device, &deviceAdressInfo);

    // create index buffer
    newSurface.indexBuffer = vk::createBuffer(
        allocator, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    vk::AllocatedBuffer staging = vk::createBuffer(
        allocator, vertexBufferSize + indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = 0;
    vmaMapMemory(allocator, staging.allocation, &data);

    // copy vertex buffer
    memcpy(data, vertices.data(), vertexBufferSize);
    // copy index buffer
    memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);
    vmaUnmapMemory(allocator, staging.allocation);

    submitter.immediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy vertexCopy{0};
        vertexCopy.dstOffset = 0;
        vertexCopy.srcOffset = 0;
        vertexCopy.size = vertexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1,
                        &vertexCopy);

        VkBufferCopy indexCopy{0};
        indexCopy.dstOffset = 0;
        indexCopy.srcOffset = vertexBufferSize;
        indexCopy.size = indexBufferSize;

        vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1,
                        &indexCopy);
    });

    vmaDestroyBuffer(allocator, staging.buffer, staging.allocation);
    newSurface.nVertices = vertices.size();
    newSurface.nIndices = indices.size();
    meshes[name] = newSurface;
    return &(meshes[name]);
}

void RendererImp::drawTriangle(VkCommandBuffer cmd) {
    // begin a render pass  connected to our draw image
    VkRenderingAttachmentInfo colorAttachment = vk::attachmentInfo(
        drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingAttachmentInfo depthAttachment = vk::depthAttachmentInfo(
        depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    VkRenderingInfo renderInfo =
        vk::renderingInfo({.width = screenSize.w, .height = screenSize.h},
                          &colorAttachment, &depthAttachment);
    vkCmdBeginRendering(cmd, &renderInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, trianglePipeline);

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
    for (const auto [k, m] : meshes) {
        GPUDrawPushConstants push_constants;
        push_constants.worldMatrix = glm::mat4{1.f};
        push_constants.vertexBuffer = m.vertexBufferAddr;

        vkCmdPushConstants(cmd, trianglePipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(GPUDrawPushConstants), &push_constants);
        vkCmdBindIndexBuffer(cmd, m.indexBuffer.buffer, 0,
                             VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(cmd, m.nIndices, 1, 0, 0, 0);
    }
    vkCmdEndRendering(cmd);
}

void RendererImp::drawBackground(VkCommandBuffer buffer) {
    VkClearColorValue clearValue;
    float flash = abs(sin(frameCounter / 120.f));
    clearValue = {{0.0f, 0.0f, flash, 1.0f}};

    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    vkCmdClearColorImage(buffer, drawImage.image, VK_IMAGE_LAYOUT_GENERAL,
                         &clearValue, 1, &clearRange);
}

RendererImp::~RendererImp() {
    vkDeviceWaitIdle(system.get().device);

    globalDescriptorAllocator.destroyPool(system.get().device);

    vkDestroyPipelineLayout(system.get().device, trianglePipelineLayout,
                            nullptr);
    vkDestroyPipeline(system.get().device, trianglePipeline, nullptr);
    vk::freeImage(depthImage, system.get().device, system.get().allocator);
    vk::freeImage(drawImage, system.get().device, system.get().allocator);
}

void RendererImp::render() {
    const auto& system = this->system.get();
    auto device = system.device;
    auto& frame = frameData.getFrame(frameCounter);
    auto renderFence = frame.renderFence;
    auto renderSemaphore = frame.renderSemaphore;
    auto swapchainSemaphore = frame.swapchainSemaphore;
    auto buffer = frame.buffer;
    vkWaitForFences(device, 1, &renderFence, true, 10000000000000);
    vkResetFences(device, 1, &renderFence);

    frame.deletion.clear(system.device, system.allocator);

    uint32_t swwapchainImageIndex;
    vkAcquireNextImageKHR(device, swapchain.getSwapchain(), 10000000000000,
                          swapchainSemaphore, 0, &swwapchainImageIndex);

    auto swapchainImage = swapchain.getImage(swwapchainImageIndex);
    auto swapchainImageView = swapchain.getImageView(swwapchainImageIndex);

    vkResetCommandBuffer(buffer, 0);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buffer, &info);

    vkCommands::transitionImage(buffer, drawImage.image,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_GENERAL);
    vkCommands::transitionImage(buffer, depthImage.image,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

    drawBackground(buffer);

    vkCommands::transitionImage(buffer, drawImage.image,
                                VK_IMAGE_LAYOUT_GENERAL,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    drawTriangle(buffer);

    // transtion the draw image and the swapchain image into their correct
    // transfer layouts
    vkCommands::transitionImage(buffer, drawImage.image,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkCommands::transitionImage(buffer, swapchainImage,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vk::copyImageToImage(buffer, drawImage.image, swapchainImage,
                         {.width = screenSize.w, .height = screenSize.h},
                         {.width = screenSize.w, .height = screenSize.h});

    vkCommands::transitionImage(buffer, swapchainImage,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    vkEndCommandBuffer(buffer);

    VkCommandBufferSubmitInfo cmdinfo = command_buffer_submit_info(buffer);

    VkSemaphoreSubmitInfo waitInfo = semaphore_submit_info(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
        swapchainSemaphore);
    VkSemaphoreSubmitInfo signalInfo = semaphore_submit_info(
        VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, renderSemaphore);

    VkSubmitInfo2 submit = submit_info(&cmdinfo, &signalInfo, &waitInfo);

    vkQueueSubmit2(system.graphicsQueue, 1, &submit, renderFence);
    auto sc = swapchain.getSwapchain();
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &sc;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swwapchainImageIndex;

    vkQueuePresentKHR(system.graphicsQueue, &presentInfo);

    frameCounter++;
}

void RendererImp::destroyMesh(const std::string& name) {
    if (!meshes.contains(name)) {
        throw new std::runtime_error("Cannot delete nonexisting mesh");
    }
    auto mesh = meshes[name];
    frameData.getFrame(frameCounter + 1).deletion.addBuffer(mesh.indexBuffer);
    frameData.getFrame(frameCounter + 1).deletion.addBuffer(mesh.vertexBuffer);
    meshes.erase(name);
}

void RendererImp::initDescriptorSets() {
    auto device = system.get().device;
    std::vector<vk::DescriptorAllocator::PoolSizeRatio> sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

    globalDescriptorAllocator.initPool(device, 10, sizes);
}

RendererImp::System::System(SDL_Window* win) { vk::initSystem(system, win); }

RendererImp::System::~System() { vk::destroySystem(system); }

}  // namespace vkr