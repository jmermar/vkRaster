#pragma once
#include <SDL3/SDL.h>

#include <cstdint>
#include <glm/glm.hpp>

#include "../types.hpp"
namespace vk {
struct vkApp;
}

namespace vkr {
class BufferWritter;
class SceneState;
class UnlitPass;
class Renderer {
   private:
    vk::vkApp& app;
    BufferWritter* bufferWritter;
    SceneState* sceneState;
    UnlitPass* unlitPass;
    Size screenSize;

   public:
    Renderer(SDL_Window* window, uint32_t w, uint32_t h);
    ~Renderer();

    void render(glm::vec4 color);
};
}  // namespace vkr