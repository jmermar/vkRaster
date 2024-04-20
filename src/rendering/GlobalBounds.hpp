#pragma once
#include <vector>

#include "vk/vkApp.hpp"
#include "vk/vkDescriptors.hpp"
namespace vkr {
enum StorageBind : uint32_t;
enum UniformBind : uint32_t;
enum TextureBind : uint32_t;
class SceneState;
class GlobalBounds {
    friend class SceneState;

   private:
    vk::vkApp& app;
    VkSampler nearestSampler;
    VkSampler linearSampler;
    std::vector<bool> textureBounds;

    vk::DescriptorAllocator descAlloc;
    VkDescriptorSetLayout descLayout;
    VkDescriptorSet descriptor;
    std::vector<bool> storageBounds;
    std::vector<bool> uniformBounds;

    GlobalBounds();

   public:
    enum SamplerType { SAMPLER_LINEAR, SAMPLER_NEAREST };
    ~GlobalBounds();
    UniformBind bindUniform(VkBuffer buffer);
    StorageBind bindStorage(VkBuffer buffer);
    TextureBind bindTexture(VkImageView image, SamplerType type);

    void removeBind(TextureBind bind);
    void removeBind(UniformBind bind);
    void removeBind(StorageBind bind);

    const VkDescriptorSet& getDescriptor() { return descriptor; }
    const VkDescriptorSetLayout& getLayout() { return descLayout; }
};
}  // namespace vkr