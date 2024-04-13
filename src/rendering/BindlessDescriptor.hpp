#pragma once
#include "vk/vkDescriptors.hpp"

namespace vkr {
class Renderer;

enum BufferBindHandle : uint32_t;
enum UniformBindHandle : uint32_t;

class GlobalDescriptors {
   private:
    Renderer& render;
    vk::DescriptorAllocator alloc;

    std::vector<bool> storageBounds;
    std::vector<bool> uniformBounds;

    size_t getFirstFree(std::vector<bool>& v);

   public:
    GlobalDescriptors(Renderer& render);
    ~GlobalDescriptors();

    VkDescriptorSet descriptor;
    VkDescriptorSetLayout layout;

    BufferBindHandle addStorageBuffer(VkBuffer buffer);
    UniformBindHandle addUniformBuffer(VkBuffer buffer);

    void removeBind(BufferBindHandle handle);
    void removeBind(UniformBindHandle handle);
};
}  // namespace vkr