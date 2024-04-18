#include "vkDescriptors.hpp"

#include "vkUtils.hpp"

namespace vk {
void DescriptorLayoutBuilder::addBinding(uint32_t binding,
                                         VkDescriptorType type,
                                         size_t descriptorCount,
                                         VkDescriptorBindingFlags flags) {
    VkDescriptorSetLayoutBinding newbind{};
    newbind.binding = binding;
    newbind.descriptorCount = descriptorCount;
    newbind.descriptorType = type;

    this->flags.push_back(flags);

    bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear() { bindings.clear(); }

VkDescriptorSetLayout DescriptorLayoutBuilder::build(
    VkDevice device, VkDescriptorSetLayoutCreateFlags flags,
    VkShaderStageFlags shaderStages) {
    for (auto& b : bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    info.pNext = nullptr;

    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = flags;

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.pNext = nullptr;
    bindingFlags.pBindingFlags = this->flags.data();
    bindingFlags.bindingCount = this->flags.size();

    info.pNext = &bindingFlags;

    VkDescriptorSetLayout set;
    VK_TRY(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));

    return set;
}

void DescriptorAllocator::initPool(VkDevice device, std::span<PoolSize> sizes,
                                   VkDescriptorPoolCreateFlags flags) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    size_t size = 0;
    for (PoolSize ratio : sizes) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type, .descriptorCount = uint32_t(ratio.size)});
        size += ratio.size;
    }

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = flags;
    pool_info.maxSets = size;
    pool_info.poolSizeCount = (uint32_t)poolSizes.size();
    pool_info.pPoolSizes = poolSizes.data();

    VK_TRY(vkCreateDescriptorPool(device, &pool_info, nullptr, &pool));
}

void DescriptorAllocator::clearDescriptors(VkDevice device) {
    VK_TRY(vkResetDescriptorPool(device, pool, 0));
}

void DescriptorAllocator::destroyPool(VkDevice device) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device,
                                              VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet ds;
    VK_TRY(vkAllocateDescriptorSets(device, &allocInfo, &ds));

    return ds;
}

}  // namespace vk