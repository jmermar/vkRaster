#include "GlobalBounds.hpp"

const uint32_t UNIFORM_BIND = 0;
const uint32_t STORAGE_BIND = 1;
const uint32_t IMAGE_SAMPLER_BIND = 2;

size_t getFirstFree(std::vector<bool>& v) {
    int i = 0;
    for (auto b : v) {
        if (!b) {
            v[i] = true;
            return i + 1;
        }
        i++;
    }
    v.push_back(true);
    return v.size();
}

namespace vkr {
GlobalBounds::GlobalBounds() : app(vk::vkApp::get()) {
    vk::DescriptorLayoutBuilder layoutBuilder;
    auto& limits = app.system.physicalDeviceProperties.limits;
    layoutBuilder.addBinding(UNIFORM_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             limits.maxDescriptorSetUniformBuffers,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(STORAGE_BIND, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             limits.maxDescriptorSetStorageBuffers,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(IMAGE_SAMPLER_BIND,
                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             limits.maxDescriptorSetSampledImages,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

    descLayout = layoutBuilder.build(
        app.system.device,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        VK_SHADER_STAGE_ALL);

    vk::DescriptorAllocator::PoolSize sizes[3] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024}};

    descAlloc.initPool(app.system.device, sizes,
                       VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    descriptor = descAlloc.allocate(app.system.device, descLayout);

    VkSamplerCreateInfo sampl = {.sType =
                                     VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    sampl.magFilter = VK_FILTER_NEAREST;
    sampl.minFilter = VK_FILTER_NEAREST;

    vkCreateSampler(app.system.device, &sampl, nullptr, &nearestSampler);

    sampl.magFilter = VK_FILTER_LINEAR;
    sampl.minFilter = VK_FILTER_LINEAR;
}
GlobalBounds::~GlobalBounds() {
    vkDestroySampler(app.system.device, nearestSampler, 0);
    vkDestroySampler(app.system.device, linearSampler, 0);

    descAlloc.destroyPool(app.system.device);
    vkDestroyDescriptorSetLayout(app.system.device, descLayout, 0);
}
UniformBind GlobalBounds::bindUniform(VkBuffer buffer) {
    size_t newHandle = getFirstFree(uniformBounds);

    VkWriteDescriptorSet write = {.sType =
                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    write.dstSet = descriptor;
    write.dstBinding = UNIFORM_BIND;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.dstArrayElement = newHandle;
    write.pBufferInfo = &bufferInfo;

    write.dstBinding = 1;

    vkUpdateDescriptorSets(app.system.device, 1, &write, 0, nullptr);

    return (UniformBind)newHandle;
}
StorageBind GlobalBounds::bindStorage(VkBuffer buffer) {
    size_t newHandle = getFirstFree(storageBounds);

    VkWriteDescriptorSet write = {.sType =
                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    write.dstSet = descriptor;
    write.dstBinding = STORAGE_BIND;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.dstArrayElement = newHandle;
    write.pBufferInfo = &bufferInfo;

    write.dstBinding = 1;

    vkUpdateDescriptorSets(app.system.device, 1, &write, 0, nullptr);

    return (StorageBind)newHandle;
}
TextureBind GlobalBounds::bindTexture(VkImageView image, SamplerType type) {
    VkSampler sampler;
    switch (type) {
        case SAMPLER_LINEAR:
            sampler = linearSampler;
            break;
        case SAMPLER_NEAREST:
            sampler = nearestSampler;
            break;
    }

    VkDescriptorImageInfo info = VkDescriptorImageInfo{
        .sampler = sampler,
        .imageView = image,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

    VkWriteDescriptorSet write = {.sType =
                                      VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

    auto bind = getFirstFree(textureBounds);

    write.dstBinding = IMAGE_SAMPLER_BIND;
    write.dstSet = descriptor;  // left empty for now until we need to write it
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &info;
    write.dstArrayElement = bind;
    vkUpdateDescriptorSets(app.system.device, 1, &write, 0, 0);

    return (TextureBind)bind;
}
void GlobalBounds::removeBind(TextureBind bind) {
    if (!bind) return;
    textureBounds[bind - 1] = false;
}
void GlobalBounds::removeBind(UniformBind bind) {
    if (!bind) return;
    uniformBounds[bind - 1] = false;
}
void GlobalBounds::removeBind(StorageBind bind) {
    if (!bind) return;
    storageBounds[bind - 1] = false;
}
void GlobalBounds::clearBounds() {
    storageBounds.clear();
    textureBounds.clear();
    uniformBounds.clear();
}
}  // namespace vkr