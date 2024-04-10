#include "vkSync.hpp"

bool vk::semaphoreCreate(VkSemaphore& semaphore, VkDevice device,
                         VkSemaphoreCreateFlags flags) {
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = nullptr;

    info.flags = flags;

    return vkCreateSemaphore(device, &info, 0, &semaphore) == VK_SUCCESS;
}

bool vk::fenceCreate(VkFence& fence, VkDevice device,
                     VkFenceCreateFlags flags) {
    VkFenceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.pNext = nullptr;

    info.flags = flags;

    return vkCreateFence(device, &info, 0, &fence) == VK_SUCCESS;
}
