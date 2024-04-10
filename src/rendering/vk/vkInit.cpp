#include "vkInit.hpp"

#include <SDL2/SDL_vulkan.h>
#include <VkBootstrap.h>
constexpr bool bUseValidationLayers = true;
bool vk::initSystem(vkSystem& system, SDL_Window* win) {
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("Example Vulkan Application")
                        .request_validation_layers(bUseValidationLayers)
                        .use_default_debug_messenger()
                        .require_api_version(1, 3, 0)
                        .build();

    vkb::Instance vkb_inst = inst_ret.value();
    system.instance = vkb_inst.instance;
    system.debug_messenger = vkb_inst.debug_messenger;

    SDL_Vulkan_CreateSurface(win, system.instance, &system.surface);
    VkPhysicalDeviceVulkan13Features features{};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physicalDevice =
        selector.set_minimum_version(1, 3)
            .set_required_features_13(features)
            .set_required_features_12(features12)
            .set_surface(system.surface)
            .select()
            .value();

    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();

    system.device = vkbDevice.device;
    system.chosenGPU = physicalDevice.physical_device;

    system.graphicsQueue =
        vkbDevice.get_queue(vkb::QueueType::graphics).value();
    system.presentQueue = vkbDevice.get_queue(vkb::QueueType::present).value();

    system.graphicsQueueFamily =
        vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    system.presentQueueFamily =
        vkbDevice.get_queue_index(vkb::QueueType::present).value();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = system.chosenGPU;
    allocatorInfo.device = system.device;
    allocatorInfo.instance = system.instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &system.allocator);

    return true;
}

bool vk::destroySystem(vkSystem& system) {
    if (system.allocator) {
        vmaDestroyAllocator(system.allocator);
        system.allocator = 0;
    }
    if (system.surface) {
        vkDestroySurfaceKHR(system.instance, system.surface, 0);
        system.surface = 0;
    }

    if (system.device) {
        vkDestroyDevice(system.device, 0);
    }

    if (system.debug_messenger) {
        vkb::destroy_debug_utils_messenger(system.instance,
                                           system.debug_messenger);
        system.debug_messenger = 0;
    }

    if (system.instance) {
        vkDestroyInstance(system.instance, 0);
        system.instance = 0;
    }

    return 1;
}
