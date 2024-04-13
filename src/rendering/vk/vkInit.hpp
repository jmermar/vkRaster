#pragma once

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>
#include "vma/vk_mem_alloc.h"
#include <vector>

namespace vk {
struct vkSystem {
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debug_messenger{};
    VkPhysicalDevice chosenGPU{};
    VkDevice device{};
    VkSurfaceKHR surface{};

    VmaAllocator allocator{};

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;

    VkPhysicalDeviceProperties physicalDeviceProperties;
};

bool initSystem(vkSystem& system, SDL_Window* win);
bool destroySystem(vkSystem& system);
}  // namespace vk