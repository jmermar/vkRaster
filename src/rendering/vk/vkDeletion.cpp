#include "vkDeletion.hpp"

namespace vk {
void DeletionQueue::clear(VkDevice device, VmaAllocator alloc) {
    for (auto& image : images) {
        if (image.imageView) {
            vkDestroyImageView(device, image.imageView, nullptr);
        }
        if (image.alloc) {
            vmaDestroyImage(alloc, image.image, image.alloc);
        } else if (image.image) {
            vkDestroyImage(device, image.image, nullptr);
        }
    }
    images.clear();

    for (auto& buffer : buffers) {
        if (buffer.alloc) {
            vmaDestroyBuffer(alloc, buffer.buffer, buffer.alloc);
        } else if (buffer.buffer) {
            vkDestroyBuffer(device, buffer.buffer, nullptr);
        }
    }
    buffers.clear();
}
};  // namespace vk
