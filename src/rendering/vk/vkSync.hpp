#pragma once
#include "vulkan/vulkan.h"


namespace vk {
bool semaphoreCreate(VkSemaphore& semaphore, VkDevice device, VkSemaphoreCreateFlags flags);
bool fenceCreate(VkFence& fence, VkDevice device, VkFenceCreateFlags flags);
};