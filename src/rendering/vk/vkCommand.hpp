#pragma once
#include <vulkan/vulkan.h>
VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask);
namespace vkCommands {
void transitionImage(VkCommandBuffer cmd, VkImage image,
                     VkImageLayout currentLayout, VkImageLayout newLayout);
}