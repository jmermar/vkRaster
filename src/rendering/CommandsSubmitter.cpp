#include "CommandsSubmitter.hpp"
#include "vk/vkSync.hpp"
namespace vkr {
CommandsSubmitter::CommandsSubmitter(const vk::vkSystem& system): system(system) {
    VkCommandPoolCreateInfo commandPoolInfo = {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = system.graphicsQueueFamily;
    vkCreateCommandPool(system.device, &commandPoolInfo, nullptr, &pool);

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = pool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(system.device, &cmdAllocInfo, &buffer);

    vk::fenceCreate(fence, system.device, VK_FENCE_CREATE_SIGNALED_BIT);
}
CommandsSubmitter::~CommandsSubmitter() {
    vkDestroyFence(system.device, fence, nullptr);
    vkDestroyCommandPool(system.device, pool, nullptr);
}
void CommandsSubmitter::immediateSubmit(
    std::function<void(VkCommandBuffer cmd)>&& function) {
    vkResetFences(system.device, 1, &fence);
    vkResetCommandBuffer(buffer, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;

    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buffer, &beginInfo);

    function(buffer);

    vkEndCommandBuffer(buffer);

    VkCommandBufferSubmitInfo bufferSubmit{};
    bufferSubmit.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    bufferSubmit.pNext = nullptr;
    bufferSubmit.commandBuffer = buffer;
    bufferSubmit.deviceMask = 0;

    VkSubmitInfo2 info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    info.pNext = nullptr;

    info.waitSemaphoreInfoCount = 0;
    info.pWaitSemaphoreInfos = 0;

    info.signalSemaphoreInfoCount = 0;
    info.pSignalSemaphoreInfos = 0;

    info.commandBufferInfoCount = 1;
    info.pCommandBufferInfos = &bufferSubmit;

    vkQueueSubmit2(system.graphicsQueue, 1, &info, fence);

    vkWaitForFences(system.device, 1, &fence, true, 9999999999);
}
}  // namespace vkr