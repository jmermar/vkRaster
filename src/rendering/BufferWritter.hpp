#pragma once

#include "vk/vkApp.hpp"

namespace vkr {
class Renderer;
class BufferWritter {
    friend class Renderer;

   private:
    static BufferWritter* instance;

    vk::vkApp& app;
    vk::vkSystem& system;

    BufferWritter();

   public:
    ~BufferWritter();

    void writeBuffer(void* data, int size, VkBuffer buffer);
    void writeImage(void* data, VkImage image, VkExtent3D size);

    static BufferWritter& get() { return *instance; }
};
}  // namespace vkr