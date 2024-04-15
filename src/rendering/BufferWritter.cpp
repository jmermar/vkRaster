#include "BufferWritter.hpp"

#include "vk/vkBuffer.hpp"
#include "vk/vkSync.hpp"

namespace vkr {
BufferWritter* BufferWritter::instance = 0;

BufferWritter::BufferWritter() : app(vk::vkApp::get()), system(app.system) {
    BufferWritter::instance = this;
}

BufferWritter::~BufferWritter() {}

void BufferWritter::writeImage(void* data, VkImage image, VkExtent3D size) {
    auto device = system.device;
    auto allocator = system.allocator;

    size_t data_size = size.depth * size.width * size.height * 4;
    vk::AllocatedBuffer uploadbuffer =
        vk::createBuffer(allocator, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VMA_MEMORY_USAGE_CPU_TO_GPU);

    memcpy(uploadbuffer.info.pMappedData, data, data_size);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = size;

    app.copyImages.push_back(
        {.copy = copyRegion, .staging = uploadbuffer, .dest = image});
}

void BufferWritter::writeBuffer(void* data, int size, VkBuffer bufferToWrite) {
    const auto allocator = app.system.allocator;
    const auto device = app.system.device;

    vk::AllocatedBuffer staging =
        vk::createBuffer(allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VMA_MEMORY_USAGE_CPU_ONLY);

    void* mapData = 0;
    vmaMapMemory(allocator, staging.allocation, &mapData);

    memcpy(mapData, data, size);
    vmaUnmapMemory(allocator, staging.allocation);

    VkBufferCopy copy{0};
    copy.dstOffset = 0;
    copy.srcOffset = 0;
    copy.size = size;

    app.copyBuffers.push_back(
        {.copy = copy, .staging = staging, .dest = bufferToWrite});
}
}  // namespace vkr
