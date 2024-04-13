#pragma once
#include "vk/vkInit.hpp"
#include "vk/vkSync.hpp"
#include "vk/vkDeletion.hpp"

constexpr size_t FRAMES_IN_FLIGHT = 2;

struct FrameStructs {
    VkCommandPool pool{};
    VkCommandBuffer buffer{};

    VkDeviceMemory depthDeviceMemory{};

    VkSemaphore swapchainSemaphore{}, renderSemaphore{};
    VkFence renderFence{};

    vk::DeletionQueue deletion;
};

class FrameData {
   private:
    const vk::vkSystem* system{};
    FrameStructs frames[FRAMES_IN_FLIGHT];

    void loadFrame(FrameStructs& frame);

   public:
    FrameData(const vk::vkSystem& system);
    FrameData() = default;
    ~FrameData();

    void init(const vk::vkSystem& system);
    void free();

    FrameData(const FrameData&) = delete;
    FrameData& operator=(const FrameData&) = delete;

    FrameStructs& getFrame(uint32_t frame) {
        return frames[frame % FRAMES_IN_FLIGHT];
    }
};