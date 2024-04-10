#pragma once

#include <vector>

#include "vk/vkInit.hpp"

class Swapchain {
   private:
    const vk::vkSystem* system{};

    std::vector<VkImage> images{};
    std::vector<VkImageView> imageViews{};
    VkSwapchainKHR swapchain{};

    VkExtent2D extent{};

   public:
    Swapchain(const vk::vkSystem& system, uint32_t w, uint32_t h);
    Swapchain() = default;
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    Swapchain(Swapchain&& other) noexcept : system(other.system) {
        images = std::move(other.images);
        imageViews = std::move(other.imageViews);
        swapchain = other.swapchain;
        extent = other.extent;
        system = other.system;

        other.swapchain = 0;
        other.extent = {.width = 0, .height = 0};
    }
    Swapchain& operator=(Swapchain&& other) noexcept {
        destroy();

        images = std::move(other.images);
        imageViews = std::move(other.imageViews);
        swapchain = other.swapchain;
        extent = other.extent;
        system = other.system;

        other.swapchain = 0;
        other.extent = {.width = 0, .height = 0};

        return *this;
    }

    void init(const vk::vkSystem& system, uint32_t w, uint32_t h);
    void destroy();

    inline VkExtent2D getExtent() { return extent; }
    inline VkSwapchainKHR getSwapchain() { return swapchain; }
    inline VkImage getImage(uint32_t index) { return images[index]; }
    inline VkImageView getImageView(uint32_t index) {
        return imageViews[index];
    }
};