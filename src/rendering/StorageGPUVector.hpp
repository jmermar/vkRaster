#pragma once

#include <span>
#include <stdexcept>

#include "GlobalBounds.hpp"
#include "vk/vkApp.hpp"
namespace vkr {
enum StorageBind : uint32_t;

template <typename T>
class StorageGPUVector {
   private:
    vk::vkApp& app;
    vk::AllocatedBuffer buffer{};
    StorageBind bindPoint{};
    GlobalBounds* bounds;

    std::vector<T> data;
    bool dirty{};

    VkBufferUsageFlags usage;
    bool bind{};

    size_t initialSize;

   public:
    using Handle = uint32_t;
    StorageGPUVector(VkBufferUsageFlags usage,
                     GlobalBounds& bounds = *((GlobalBounds*)0),
                     size_t initialSize = 0)
        : usage(usage),
          dirty(true),
          app(vk::vkApp::get()),
          bounds(&bounds),
          initialSize(initialSize) {
        data.resize(initialSize);
        bind = this->bounds;
    }
    ~StorageGPUVector() { app.deletion.addBuffer(buffer); }

    void update() {
        if (dirty) {
            dirty = false;
            app.deletion.addBuffer(buffer);
            if (bind) {
                bounds->removeBind(bindPoint);
                bindPoint = (StorageBind)0;
            }
            if (data.size() > 0 || initialSize > 0) {
                size_t size =
                    sizeof(T) * (initialSize > 0 ? initialSize : data.size());
                buffer = vk::createBuffer(
                    app.system.allocator, size,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | usage,
                    VMA_MEMORY_USAGE_GPU_ONLY);
                BufferWritter::get().writeBuffer(data.data(), size,
                                                 buffer.buffer);
                if (bind) {
                    bindPoint = bounds->bindStorage(buffer.buffer);
                }
            }
        }
    }

    Handle add(const T& elem) {
        data.push_back(elem);
        dirty = true;
        return data.size() - 1;
    }

    void clear() {
        data.clear();
        dirty = true;
    }

    inline size_t getSize() { return data.size(); }

    void updateElem(Handle elem, const T& content) {
        if (elem >= data.size()) {
            throw std::runtime_error(
                "Illegal element at StorageGPUVector.updateElem");
        }
        data[elem] = content;
        dirty = true;
    }

    void insert(const std::span<const T>& elems) {
        data.insert(data.end(), elems.begin(), elems.end());
        dirty = true;
    }

    inline bool hasData() { return data.size() > 0; }

    inline const VkBuffer& getBuffer() { return buffer.buffer; }
    inline StorageBind getBindPoint() { return bindPoint; }
};
}  // namespace vkr