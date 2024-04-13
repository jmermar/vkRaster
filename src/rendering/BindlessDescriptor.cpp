#include "BindlessDescriptor.hpp"

#include "Renderer.hpp"

constexpr uint32_t UNIFORM_BIND = 0;
constexpr uint32_t STORAGE_BIND = 1;
constexpr uint32_t IMAGE_SAMPLER_BIND = 2;

namespace vkr {
size_t GlobalDescriptors::getFirstFree(std::vector<bool>& v) {
    int i = 0;
    for (auto b : v) {
        if (!b) {
            v[i] = true;
            return i;
        }
        i++;
    }
    v.push_back(true);
    return i;
}
GlobalDescriptors::GlobalDescriptors(Renderer& render) : render(render) {
    vk::DescriptorLayoutBuilder layoutBuilder;
    layoutBuilder.addBinding(UNIFORM_BIND, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                             1000,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(STORAGE_BIND, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             1000,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
    layoutBuilder.addBinding(IMAGE_SAMPLER_BIND,
                             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000,
                             VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                 VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);

    layout = layoutBuilder.build(
        render.getSystem().device,
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        VK_SHADER_STAGE_ALL);

    vk::DescriptorAllocator::PoolSizeRatio sizes[3] = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1.0},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1.0},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1.0}};

    alloc.initPool(render.getSystem().device, 1009, sizes,
                   VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT);

    descriptor = alloc.allocate(render.getSystem().device, layout);
}
GlobalDescriptors::~GlobalDescriptors() {
    alloc.destroyPool(render.getSystem().device);
    vkDestroyDescriptorSetLayout(render.getSystem().device, layout, nullptr);
}
BufferBindHandle GlobalDescriptors::addStorageBuffer(VkBuffer buffer) {
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

    vkUpdateDescriptorSets(render.getSystem().device, 1, &write, 0, nullptr);

    return (BufferBindHandle)newHandle;
}

UniformBindHandle GlobalDescriptors::addUniformBuffer(VkBuffer buffer) {
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

    vkUpdateDescriptorSets(render.getSystem().device, 1, &write, 0, nullptr);

    return (UniformBindHandle)newHandle;
}
void GlobalDescriptors::removeBind(BufferBindHandle handle) {
    storageBounds[handle] = false;
}
void GlobalDescriptors::removeBind(UniformBindHandle handle) {
    uniformBounds[handle] = false;
}
}  // namespace vkr