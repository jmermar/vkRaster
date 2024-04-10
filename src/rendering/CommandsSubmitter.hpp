#pragma once
#include <functional>

#include "vk/vkInit.hpp"
namespace vkr {
class CommandsSubmitter {
   private:
    VkCommandPool pool;
    const vk::vkSystem& system;
    VkCommandBuffer buffer;
    VkFence fence;

   public:
    CommandsSubmitter(const vk::vkSystem& system);
    ~CommandsSubmitter();

    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
};
}  // namespace vkr