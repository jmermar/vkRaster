#pragma once
#include "types.hpp"

namespace vkr {
struct GlobalRenderData {
    Size windowSize;
    bool frustumCulling;
    glm::vec3 clearColor;
    CameraData camera;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    bool sizeChanged;

    static GlobalRenderData& get() {
        static GlobalRenderData ins;
        return ins;
    }
};
}  // namespace vkr