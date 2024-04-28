#pragma once
#include <functional>

#include "types.hpp"
#include "vkDeletion.hpp"
#include "vkImage.hpp"
#include "vkInit.hpp"
namespace vk {
constexpr size_t FRAMES_IN_FLIGHT = 2;
struct vkApp {
    struct FrameData {
        VkCommandPool pool{};
        VkCommandBuffer buffer{};

        VkDeviceMemory depthDeviceMemory{};

        VkSemaphore swapchainSemaphore{}, renderSemaphore{};
        VkFence renderFence{};

        vk::DeletionQueue deletion{};
    };

    struct Swapchain {
        std::vector<VkImage> images{};
        std::vector<VkImageView> imageViews{};
        VkSwapchainKHR swapchain{};
    };

    struct CopyBufferCommand {
        VkBufferCopy copy;
        vk::AllocatedBuffer staging;
        VkBuffer dest;
        uint32_t size;
    };

    struct CopyImageCommand {
        VkBufferImageCopy copy;
        vk::AllocatedBuffer staging;
        VkImage dest;
    };

    std::vector<CopyBufferCommand> copyBuffers;
    std::vector<CopyImageCommand> copyImages;

    VkFence immFence;
    VkCommandBuffer immCommandBuffer;
    VkCommandPool immCommandPool;

    SDL_Window* window{};
    vkSystem system{};
    FrameData frameData[FRAMES_IN_FLIGHT]{};
    Swapchain swapchain{};

    AllocatedImage drawImage{};
    AllocatedImage depthImage{};

    VkDescriptorPool imguiPool;

    bool shouldRegenerate{};

    static vkApp& get() {
        static vkApp instance;
        return instance;
    }

    void init(SDL_Window* window, size_t w, size_t h);
    void finish();

    bool renderBegin(FrameData** frame);
    void renderEnd();

    inline uint32_t getFrameIndex() { return frameCounter % FRAMES_IN_FLIGHT; }

    DeletionQueue deletion;

    const vkr::Size& getScreenSize() { return screenSize; }

    void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

   private:
    void regenerate();

    void initImgui();
    void initCommands();

    void initSwapchain(size_t w, size_t h);
    void initFrameData();
    void initImages(size_t w, size_t h);

    void destroyCommands();
    void destroyImages();
    void destroySwapchain();
    void destroyFrameData();

    void prepareUploads(VkCommandBuffer cmd);

    vkr::Size screenSize;

    uint32_t frameCounter{};
    uint32_t swapchainImageIndex = 0;
};
}  // namespace vk