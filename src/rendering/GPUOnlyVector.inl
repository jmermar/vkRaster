#pragma once

#include <span>
#include <stdexcept>

#include "GlobalBounds.hpp"
#include "vk/vkApp.hpp"
namespace vkr {
enum StorageBind : uint32_t;

template <typename T>
class GPUOnlyVector {
   private:
    vk::vkApp& app;
    vk::AllocatedBuffer buffer{};
    StorageBind bindPoint{};
    GlobalBounds* bounds;
    VkBufferUsageFlags usage = {};
    bool bind{};
    size_t size;
    bool dirty{true};

   public:
    using Handle = uint32_t;
    GPUOnlyVector(VkBufferUsageFlags usage, size_t size,
                  GlobalBounds* bounds = 0)
        : app(vk::vkApp::get()),
          bounds(bounds),
          usage(usage),
          bind(bounds != 0),
          size(size) {}

    GPUOnlyVector(VkBufferUsageFlags usage, size_t size, GlobalBounds& bounds)
        : GPUOnlyVector(usage, size, &bounds){};
    ~GPUOnlyVector() {
        if (bind) {
            bounds->removeBind(bindPoint);
        }
        app.deletion.addBuffer(buffer);
    }

    void update() {
        if (dirty) {
            dirty = false;

            app.deletion.addBuffer(buffer);
            size_t size = sizeof(T) * this->size;
            buffer =
                vk::createBuffer(app.system.allocator, size,
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usage,
                                 VMA_MEMORY_USAGE_GPU_ONLY);
            if (bind) {
                bounds->removeBind(bindPoint);
                bindPoint = bounds->bindStorage(buffer.buffer);
            }
        }
    }

    inline size_t getSize() { return size; }
    inline void resize(size_t newSize) {
        size = newSize;
        dirty = true;
    }

    inline const VkBuffer& getBuffer() { return buffer.buffer; }
    inline StorageBind getBindPoint() { return bindPoint; }
};
}  // namespace vkr