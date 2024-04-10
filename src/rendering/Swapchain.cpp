#include "Swapchain.hpp"

#include <VkBootstrap.h>
void Swapchain::destroy() {
    if (swapchain) {
        vkDestroySwapchainKHR(system->device, swapchain, 0);
        swapchain = 0;
    }

    for (int i = 0; i < imageViews.size(); i++) {
        vkDestroyImageView(system->device, imageViews[i], nullptr);
    }

    imageViews.clear();
    images.clear();
}

Swapchain::Swapchain(const vk::vkSystem& system, uint32_t w, uint32_t h) {
    init(system, w, h);
}

void Swapchain::init(const vk::vkSystem& system, uint32_t w, uint32_t h) {
    destroy();
    this->system = &system;
    extent = {.width = w, .height = h};
    auto gpu = system.chosenGPU;
    auto device = system.device;
    auto surface = system.surface;
    vkb::SwapchainBuilder swapchainBuilder{gpu, device, surface};

    vkb::Swapchain vkbSwapchain =
        swapchainBuilder
            .set_desired_format(VkSurfaceFormatKHR{
                .format = VK_FORMAT_B8G8R8A8_UNORM,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(w, h)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build()
            .value();
    swapchain = vkbSwapchain.swapchain;
    images = vkbSwapchain.get_images().value();
    imageViews = vkbSwapchain.get_image_views().value();
}

Swapchain::~Swapchain() { destroy(); }
