#include "FrameData.hpp"

void FrameData::loadFrame(FrameStructs& frame) {
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = system->graphicsQueueFamily;
    vkCreateCommandPool(system->device, &commandPoolInfo, nullptr, &frame.pool);

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = frame.pool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(system->device, &cmdAllocInfo, &frame.buffer);

    vk::fenceCreate(frame.renderFence, system->device,
                    VK_FENCE_CREATE_SIGNALED_BIT);
    vk::semaphoreCreate(frame.renderSemaphore, system->device, 0);
    vk::semaphoreCreate(frame.swapchainSemaphore, system->device, 0);
}
FrameData::FrameData(const vk::vkSystem& system) {
    init(system);
}

FrameData::~FrameData() { free(); }

void FrameData::init(const vk::vkSystem& system) {
    free();
    this->system = &system;

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        loadFrame(frames[i]);
    }
}

void FrameData::free() {
    if (!system) return;
    auto device = system->device;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (frames[i].pool) {
            frames[i].deletion.clear(device, system->allocator);
            if (frames[i].renderSemaphore) {
                vkDestroySemaphore(device, frames[i].renderSemaphore, 0);
                frames[i].renderSemaphore = 0;
            }

            if (frames[i].swapchainSemaphore) {
                vkDestroySemaphore(device, frames[i].swapchainSemaphore, 0);
                frames[i].swapchainSemaphore = 0;
            }
            if (frames[i].renderFence) {
                vkDestroyFence(device, frames[i].renderFence, 0);
                frames[i].renderFence = 0;
            }
            if (frames[i].pool) {
                vkDestroyCommandPool(device, frames[i].pool, 0);
                frames[i].pool = 0;
            }
        }
    }
}