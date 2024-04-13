#pragma once
#include <span>
#include <vector>

#include "vulkan/vulkan.h"

namespace vk {
struct DescriptorLayoutBuilder {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorBindingFlags> flags;

    void addBinding(uint32_t binding, VkDescriptorType type,
                    size_t descriptorCount = 1,
                    VkDescriptorBindingFlags flags = 0);
    void clear();
    VkDescriptorSetLayout build(VkDevice device,
                                VkDescriptorSetLayoutCreateFlags flags,
                                VkShaderStageFlags shaderStages);
};

struct DescriptorAllocator {
    struct PoolSizeRatio {
        VkDescriptorType type;
        size_t size;
    };

    VkDescriptorPool pool;

    void initPool(VkDevice device, std::span<PoolSizeRatio> sizes,
                  VkDescriptorPoolCreateFlags flags = 0);
    void clearDescriptors(VkDevice device);
    void destroyPool(VkDevice device);

    VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

}  // namespace vk