#pragma once
#include "vk/vkInit.hpp"
#include "vk/vkSync.hpp"
#include "vk/vkDeletion.hpp"

constexpr size_t FRAMES_IN_FLIGHT = 2;

struct FrameStructs {
    VkCommandPool pool{};
    VkCommandBuffer buffer{};

    VkImage depthImage{};
    VkImageView depthImageView{};
    VkDeviceMemory depthDeviceMemory{};

    VkSemaphore swapchainSemaphore{}, renderSemaphore{};
    VkFence renderFence{};

    vk::DeletionQueue deletion;
};

class FrameData {
   private:
    const vk::vkSystem* system{};
    FrameStructs frames[FRAMES_IN_FLIGHT];
    VkExtent2D extent{};

    void loadFrame(FrameStructs& frame);

   public:
    FrameData(const vk::vkSystem& system, uint32_t w, uint32_t h);
    FrameData() = default;
    ~FrameData();

    void init(const vk::vkSystem& system, uint32_t w, uint32_t h);
    void free();

    FrameData(const FrameData&) = delete;
    FrameData& operator=(const FrameData&) = delete;

    FrameStructs& getFrame(uint32_t frame) {
        return frames[frame % FRAMES_IN_FLIGHT];
    }
};