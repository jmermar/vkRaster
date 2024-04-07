#pragma once
#include <stdint.h>
namespace vkr {
    struct Size {
        uint32_t w, h;
    };

    struct Rect {
        int32_t x, y, w, h;
    };

    struct URect {
        uint32_t x, y, w, h;
    };

    struct IPoint {
        int32_t x, y;
    };
}