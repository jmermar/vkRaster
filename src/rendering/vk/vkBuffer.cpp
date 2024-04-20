#include "vkBuffer.hpp"

#include "vkUtils.hpp"

namespace vk {
AllocatedBuffer createBuffer(VmaAllocator allocator, size_t allocSize,
                             VkBufferUsageFlags usage,
                             VmaMemoryUsage memoryUsage) {
    VkBufferCreateInfo bufferInfo = {.sType =
                                         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = memoryUsage;
    vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    AllocatedBuffer newBuffer;

    VK_TRY(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
                           &newBuffer.buffer, &newBuffer.allocation,
                           &newBuffer.info));

    newBuffer.size = allocSize;

    return newBuffer;
}
}  // namespace vk