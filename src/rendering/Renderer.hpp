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
class PBRPass;
class CullingPass;
class ImGUIPass;
class LightCullingPass;
class Renderer {
   private:
    vk::vkApp& app;
    BufferWritter* bufferWritter;
    SceneState* sceneState;
    PBRPass* pbrPass;
    LightCullingPass* lightCullingPass;
    CullingPass* cullingPass;
    ImGUIPass* imGUIPass;
    Size screenSize;

   public:
    Renderer(SDL_Window* window, uint32_t w, uint32_t h);
    ~Renderer();

    void render(glm::vec4 color);
};
}  // namespace vkr