#include "vkApp.hpp"

#include <VkBootstrap.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include "vkCommand.hpp"
#include "vkPipelines.hpp"
#include "vkSync.hpp"
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

namespace vk {
void vkApp::init(SDL_Window* window, size_t w, size_t h) {
    this->window = window;

    initSystem(system, window);

    initSwapchain(w, h);
    initImages(w, h);
    initFrameData();

    initCommands();

    initImgui();
}
void vkApp::finish() {
    vkDeviceWaitIdle(system.device);

    ImGui_ImplVulkan_Shutdown();
    vkDestroyDescriptorPool(system.device, imguiPool, nullptr);

    destroyCommands();

    destroyFrameData();
    destroyImages();
    destroySwapchain();
}
void vkApp::immediateSubmit(
    std::function<void(VkCommandBuffer cmd)>&& function) {
    vkResetFences(system.device, 1, &immFence);
    vkResetCommandBuffer(immCommandBuffer, 0);

    VkCommandBuffer cmd = immCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;

    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);

    function(cmd);

    vkEndCommandBuffer(cmd);

    VkCommandBufferSubmitInfo cmdSubmit =
        command_buffer_submit_info(immCommandBuffer);

    VkSubmitInfo2 submit = submit_info(&cmdSubmit, 0, 0);

    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    vkQueueSubmit2(system.graphicsQueue, 1, &submit, immFence);

    vkWaitForFences(system.device, 1, &immFence, true, 9999999999);
}

void vkApp::regenerate() {
    vkDeviceWaitIdle(system.device);

    destroyImages();
    destroySwapchain();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    initSwapchain(w, h);
    initImages(screenW, screenH);
    shouldRegenerate = false;
}
void vkApp::initImgui() {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(system.device, &pool_info, nullptr, &imguiPool);

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplSDL3_InitForVulkan(window);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = system.instance;
    init_info.PhysicalDevice = system.chosenGPU;
    init_info.Device = system.device;
    init_info.Queue = system.graphicsQueue;
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = FRAMES_IN_FLIGHT;
    init_info.ImageCount = FRAMES_IN_FLIGHT;
    init_info.UseDynamicRendering = true;

    init_info.PipelineRenderingCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineRenderingCreateInfo.pColorAttachmentFormats =
        &drawImage.imageFormat;

    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    ImGui_ImplVulkan_CreateFontsTexture();
}

void vkApp::initCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = system.graphicsQueueFamily;
    vkCreateCommandPool(system.device, &commandPoolInfo, nullptr,
                        &immCommandPool);

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = immCommandPool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(system.device, &cmdAllocInfo, &immCommandBuffer);

    vk::fenceCreate(immFence, system.device, VK_FENCE_CREATE_SIGNALED_BIT);
}

bool vkApp::renderBegin(FrameData** frameP) {
    if (shouldRegenerate) {
        regenerate();
    }

    auto& frame = frameData[frameCounter % FRAMES_IN_FLIGHT];
    auto renderFence = frame.renderFence;
    auto renderSemaphore = frame.renderSemaphore;
    auto swapchainSemaphore = frame.swapchainSemaphore;
    auto buffer = frame.buffer;
    vkWaitForFences(system.device, 1, &renderFence, true, 10000000000000);
    vkResetFences(system.device, 1, &renderFence);

    frame.deletion.clear(system.device, system.allocator);
    frame.deletion = std::move(deletion);

    auto result = vkAcquireNextImageKHR(system.device, swapchain.swapchain,
                                        10000000000000, swapchainSemaphore, 0,
                                        &swapchainImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        shouldRegenerate = true;
        frameCounter++;
        return false;
    }
    *frameP = &frame;

    vkResetCommandBuffer(buffer, 0);

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buffer, &info);

    prepareUploads(buffer);

    return true;
}
void vkApp::renderEnd() {
    auto& frame = frameData[frameCounter % FRAMES_IN_FLIGHT];
    auto renderFence = frame.renderFence;
    auto renderSemaphore = frame.renderSemaphore;
    auto swapchainSemaphore = frame.swapchainSemaphore;
    auto swapchainImage = swapchain.images[swapchainImageIndex];
    auto swapchainImageView = swapchain.imageViews[swapchainImageIndex];
    auto buffer = frame.buffer;

    vkCommands::transitionImage(buffer, drawImage.image,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    vkCommands::transitionImage(buffer, swapchainImage,
                                VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    vk::copyImageToImage(buffer, drawImage.image, swapchainImage,
                         {.width = screenW, .height = screenH},
                         {.width = screenW, .height = screenH});

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
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &swapchain.swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    auto presentResult = vkQueuePresentKHR(system.graphicsQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
        shouldRegenerate = true;
    }

    frameCounter++;
}
void vkApp::initSwapchain(size_t w, size_t h) {
    vkb::SwapchainBuilder swapchainBuilder{system.chosenGPU, system.device,
                                           system.surface};

    vkb::Swapchain vkbSwapchain =
        swapchainBuilder
            .set_desired_format(VkSurfaceFormatKHR{
                .format = VK_FORMAT_B8G8R8A8_UNORM,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
            .set_desired_extent(w, h)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();

    screenW = vkbSwapchain.extent.width;
    screenH = vkbSwapchain.extent.height;

    swapchain.swapchain = vkbSwapchain.swapchain;
    swapchain.images = vkbSwapchain.get_images().value();
    swapchain.imageViews = vkbSwapchain.get_image_views().value();
}
void vkApp::initFrameData() {
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        auto& frame = frameData[i];

        // Draw commands
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.pNext = nullptr;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = system.graphicsQueueFamily;
        vkCreateCommandPool(system.device, &commandPoolInfo, nullptr,
                            &frame.pool);

        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.pNext = nullptr;
        cmdAllocInfo.commandPool = frame.pool;
        cmdAllocInfo.commandBufferCount = 1;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        vkAllocateCommandBuffers(system.device, &cmdAllocInfo, &frame.buffer);

        vk::fenceCreate(frame.renderFence, system.device,
                        VK_FENCE_CREATE_SIGNALED_BIT);
        vk::semaphoreCreate(frame.renderSemaphore, system.device, 0);
        vk::semaphoreCreate(frame.swapchainSemaphore, system.device, 0);
    }
}
void vkApp::initImages(size_t w, size_t h) {
    vk::allocateImage(
        drawImage, system.device, system.allocator,
        {.width = (uint32_t)w, .height = (uint32_t)h, .depth = 1},
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    vk::allocateImage(depthImage, system.device, system.allocator,
                      {.width = (uint32_t)w, .height = (uint32_t)h, .depth = 1},
                      VK_FORMAT_D32_SFLOAT,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_DEPTH_BIT);
}
void vkApp::destroyCommands() {
    vkDestroyFence(system.device, immFence, nullptr);
    vkDestroyCommandPool(system.device, immCommandPool, nullptr);
}
void vkApp::destroyImages() {
    vk::freeImage(drawImage, system.device, system.allocator);
    vk::freeImage(depthImage, system.device, system.allocator);
}
void vkApp::destroySwapchain() {
    if (swapchain.swapchain) {
        vkDestroySwapchainKHR(system.device, swapchain.swapchain, 0);
        swapchain.swapchain = 0;
    }

    for (int i = 0; i < swapchain.imageViews.size(); i++) {
        vkDestroyImageView(system.device, swapchain.imageViews[i], nullptr);
    }

    swapchain.imageViews.clear();
    swapchain.images.clear();
}
void vkApp::destroyFrameData() {
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (frameData[i].pool) {
            frameData[i].deletion.clear(system.device, system.allocator);
            if (frameData[i].renderSemaphore) {
                vkDestroySemaphore(system.device, frameData[i].renderSemaphore,
                                   0);
                frameData[i].renderSemaphore = 0;
            }

            if (frameData[i].swapchainSemaphore) {
                vkDestroySemaphore(system.device,
                                   frameData[i].swapchainSemaphore, 0);
                frameData[i].swapchainSemaphore = 0;
            }
            if (frameData[i].renderFence) {
                vkDestroyFence(system.device, frameData[i].renderFence, 0);
                frameData[i].renderFence = 0;
            }
            if (frameData[i].pool) {
                vkDestroyCommandPool(system.device, frameData[i].pool, 0);
                frameData[i].pool = 0;
            }
        }
    }
}
void vkApp::prepareUploads(VkCommandBuffer cmd) {
    auto& queue = frameData[frameCounter % FRAMES_IN_FLIGHT].deletion;

    VkMemoryBarrier2 barrier{.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    VkDependencyInfo depInfo{};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;
    depInfo.memoryBarrierCount = 1;
    depInfo.pMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);

    for (auto& [copy, staging, buffer, size] : copyBuffers) {
        vkCmdCopyBuffer(cmd, staging.buffer, buffer, 1, &copy);

        queue.addBuffer(staging);
    }

    for (auto& [copy, staging, image] : copyImages) {
        vkCommands::transitionImage(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vkCmdCopyBufferToImage(cmd, staging.buffer, image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
        vkCommands::transitionImage(cmd, image,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        queue.addBuffer(staging);
    }

    barrier = {.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2};
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;

    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;
    depInfo.memoryBarrierCount = 1;
    depInfo.pMemoryBarriers = &barrier;

    copyBuffers.clear();
    copyImages.clear();

    vkCmdPipelineBarrier2(cmd, &depInfo);
}
}  // namespace vk