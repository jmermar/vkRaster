#pragma once
#include <vulkan/vulkan.h>

#include <stdexcept>
#define VK_TRY(command)                                \
    {                                                  \
        if (VK_SUCCESS != (command))                   \
            throw std::runtime_error("Vulkan failed"); \
    }