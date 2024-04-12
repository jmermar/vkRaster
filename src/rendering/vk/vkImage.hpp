#pragma once
#include <vulkan/vulkan.h>

#include "vma/vk_mem_alloc.h"

namespace vk {
struct AllocatedImage {
    VkImage image{};
    VkImageView imageView{};
    VmaAllocation allocation{};
    VkExtent3D imageExtent{};
    VkFormat imageFormat{};
};

bool imageCreate(VkImage& image, VkDevice device, VkFormat format,
                 VkImageUsageFlags usageFlags, VkExtent3D extent);

bool imageViewCreate(VkImageView& imageView, VkDevice device, VkFormat format,
                     VkImage image, VkImageAspectFlags aspectFlags);

bool allocateImage(AllocatedImage& allocatedImage, VkDevice device,
                   VmaAllocator allocator, const VkExtent3D imageExtent,
                   VkFormat imageFormat, VkImageUsageFlags usage,
                   VmaMemoryUsage memoryUsage,
                   VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT);

bool freeImage(AllocatedImage& image, VkDevice device, VmaAllocator allocator);
void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination,
                      VkExtent2D srcSize, VkExtent2D dstSize);

};  // namespace vk