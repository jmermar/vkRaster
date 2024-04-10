#pragma once
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vector>

#include "vkBuffer.hpp"
#include "vkImage.hpp"

namespace vk {
struct DeleteBufferData {
    VkBuffer buffer{};
    VmaAllocation alloc{};
};
struct DeleteImageData {
    VkImageView imageView{};
    VkImage image{};
    VmaAllocation alloc{};
};
struct DeletionQueue {
    std::vector<DeleteImageData> images;
    std::vector<DeleteBufferData> buffers;

    inline void addImage(const AllocatedImage& img) {
        images.push_back({.imageView = img.imageView,
                          .image = img.image,
                          .alloc = img.allocation});
    }

    inline void addBuffer(const AllocatedBuffer& buffer) {
        buffers.push_back(
            {.buffer = buffer.buffer, .alloc = buffer.allocation});
    }

    void clear(VkDevice device, VmaAllocator alloc);
};
}  // namespace vk