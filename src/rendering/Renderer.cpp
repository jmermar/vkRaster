#include "Renderer.hpp"

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

Renderer::Renderer(SDL_Window* window, uint32_t w, uint32_t h)
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

    unlitRenderer.initPipeline();
}

void Renderer::initSubmit() {}

vk::AllocatedBuffer Renderer::uploadBuffer(void* data, size_t size,
                                           VkBufferUsageFlags usage) {
    const auto allocator = system.get().allocator;
    const auto device = system.get().device;
    vk::AllocatedBuffer newBuffer =
        vk::createBuffer(allocator, size,
                         usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                         VMA_MEMORY_USAGE_GPU_ONLY);

    vk::AllocatedBuffer staging =
        vk::createBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VMA_MEMORY_USAGE_CPU_ONLY);

    void* mapData = 0;
    vmaMapMemory(allocator, staging.allocation, &mapData);

    memcpy(mapData, data, size);
    vmaUnmapMemory(allocator, staging.allocation);

    submitter.immediateSubmit([&](VkCommandBuffer cmd) {
        VkBufferCopy copy{0};
        copy.dstOffset = 0;
        copy.srcOffset = 0;
        copy.size = size;

        vkCmdCopyBuffer(cmd, staging.buffer, newBuffer.buffer, 1, &copy);
    });
    vmaDestroyBuffer(allocator, staging.buffer, staging.allocation);
    return newBuffer;
}

GPUMesh Renderer::uploadMesh(const MeshData& meshData) {
    auto& vertices = meshData.vertices;
    auto& indices = meshData.indices;

    const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);
    const auto allocator = system.get().allocator;
    const auto device = system.get().device;
    GPUMesh newSurface;

    // create vertex buffer
    newSurface.vertexBuffer = vk::createBuffer(
        allocator, vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    // find the adress of the vertex buffer
    VkBufferDeviceAddressInfo deviceAdressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = newSurface.vertexBuffer.buffer};

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
    return newSurface;
}

void Renderer::drawBackground(VkCommandBuffer buffer) {
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

Renderer::~Renderer() {
    vkDeviceWaitIdle(system.get().device);

    vk::freeImage(depthImage, system.get().device, system.get().allocator);
    vk::freeImage(drawImage, system.get().device, system.get().allocator);
}

void Renderer::render() {
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

    unlitRenderer.draw(buffer);

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

void Renderer::destroyMesh(GPUMesh& mesh) {
    frameData.getFrame(frameCounter + 1).deletion.addBuffer(mesh.indexBuffer);
    frameData.getFrame(frameCounter + 1).deletion.addBuffer(mesh.vertexBuffer);
    mesh.indexBuffer = {0, 0, 0};
    mesh.vertexBuffer = {0, 0, 0};
}

void Renderer::destroyBuffer(vk::AllocatedBuffer& buffer) {
    frameData.getFrame(frameCounter + 1).deletion.addBuffer(buffer);
    buffer = {0, 0, 0};
}

Renderer::System::System(SDL_Window* win) { vk::initSystem(system, win); }

Renderer::System::~System() { vk::destroySystem(system); }

}  // namespace vkr