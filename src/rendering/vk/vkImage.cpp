#include "vkImage.hpp"

#include "vkUtils.hpp"

namespace vk {
void imageCreate(VkImage& image, VkDevice device, VkFormat format,
                 VkImageUsageFlags usageFlags, VkExtent3D extent) {
    VkImageCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    // for MSAA. we will not be using it by default, so default it to 1 sample
    // per pixel.
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    // optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    VK_TRY(vkCreateImage(device, &info, nullptr, &image));
}

VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags,
                                  VkExtent3D extent) {
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;

    info.imageType = VK_IMAGE_TYPE_2D;

    info.format = format;
    info.extent = extent;

    info.mipLevels = 1;
    info.arrayLayers = 1;

    // for MSAA. we will not be using it by default, so default it to 1 sample
    // per pixel.
    info.samples = VK_SAMPLE_COUNT_1_BIT;

    // optimal tiling, which means the image is stored on the best gpu format
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = usageFlags;

    return info;
}

void imageViewCreate(VkImageView& imageView, VkDevice device, VkFormat format,
                     VkImage image, VkImageAspectFlags aspectFlags) {
    // build a image-view for the depth image to use for rendering
    VkImageViewCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;

    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    VK_TRY(vkCreateImageView(device, &info, nullptr, &imageView));
}

void allocateImage(AllocatedImage& allocatedImage, VkDevice device,
                   VmaAllocator allocator, const VkExtent3D imageExtent,
                   VkFormat imageFormat, VkImageUsageFlags usage,
                   VmaMemoryUsage memoryUsage, VkImageAspectFlags aspect) {
    VkExtent3D allocatedImageExtent = {imageExtent.width, imageExtent.height,
                                       1};

    // hardcoding the draw format to 32 bit float
    allocatedImage.imageFormat = imageFormat;
    allocatedImage.imageExtent = allocatedImageExtent;

    VkImageCreateInfo rimg_info = imageCreateInfo(allocatedImage.imageFormat,
                                                  usage, allocatedImageExtent);

    // for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = memoryUsage;
    rimg_allocinfo.requiredFlags =
        VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // allocate and create the image
    VK_TRY(vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo,
                          &allocatedImage.image, &allocatedImage.allocation,
                          nullptr));

    // build a image-view for the draw image to use for rendering
    imageViewCreate(allocatedImage.imageView, device,
                    allocatedImage.imageFormat, allocatedImage.image, aspect);
}

void freeImage(AllocatedImage& image, VkDevice device, VmaAllocator allocator) {
    if (image.imageView) {
        vkDestroyImageView(device, image.imageView, nullptr);
        image.imageView = 0;
    }
    if (image.image) {
        vmaDestroyImage(allocator, image.image, image.allocation);
        image.image = 0;
        image.allocation = 0;
    }
}

void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination,
                      VkExtent2D srcSize, VkExtent2D dstSize, bool upsideDown) {
    VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
                            .pNext = nullptr};

    blitRegion.srcOffsets[1].x = srcSize.width;
    blitRegion.srcOffsets[1].y = srcSize.height;
    blitRegion.srcOffsets[1].z = 1;

    if (upsideDown) {
        blitRegion.dstOffsets[0].y = dstSize.height;

        blitRegion.dstOffsets[1].x = dstSize.width;
        blitRegion.dstOffsets[1].y = 0;
        blitRegion.dstOffsets[1].z = 1;
    } else {
        blitRegion.dstOffsets[1].x = dstSize.width;
        blitRegion.dstOffsets[1].y = dstSize.height;
        blitRegion.dstOffsets[1].z = 1;
    }

    blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount = 1;
    blitRegion.srcSubresource.mipLevel = 0;

    blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount = 1;
    blitRegion.dstSubresource.mipLevel = 0;

    VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
                              .pNext = nullptr};
    blitInfo.dstImage = destination;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.srcImage = source;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.filter = VK_FILTER_LINEAR;
    blitInfo.regionCount = 1;
    blitInfo.pRegions = &blitRegion;

    vkCmdBlitImage2(cmd, &blitInfo);
}

}  // namespace vk