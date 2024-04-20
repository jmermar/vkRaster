#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vk {
struct AllocatedBuffer {
    VkBuffer buffer{};
    VmaAllocation allocation{};
    VmaAllocationInfo info{};
    size_t size{};
};

AllocatedBuffer createBuffer(VmaAllocator allocator, size_t allocSize,
                             VkBufferUsageFlags usage,
                             VmaMemoryUsage memoryUsage);

}  // namespace vk