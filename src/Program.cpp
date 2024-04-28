#include "Program.hpp"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

#include "System.hpp"
#include "rendering/GlobalRenderData.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/SceneState.hpp"
namespace vkr {
Program::Program(const Size& size, const char* name)
    : size(size),
      winName(name),
      renderer(std::make_unique<Renderer>(system.getWindow(), size.w, size.h)) {
}
Program::~Program() {}
void Program::run() {
    bool shouldQuit = false;

    Uint32 ticks = SDL_GetTicks();

    while (!shouldQuit) {
        float delta = (SDL_GetTicks() - ticks) / 1000.f;
        if ((SDL_GetWindowFlags(system.getWindow()) & SDL_WINDOW_MINIMIZED) ==
            0) {
            int w, h;
            SDL_GetWindowSize(system.getWindow(), &w, &h);
            auto& global = GlobalRenderData::get();

            size.w = w;
            size.h = h;

            global.windowSize.w = w;
            global.windowSize.h = h;

            ticks = SDL_GetTicks();
            system.handleInput(shouldQuit);

            onFrame(delta);

            scene.update();

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            onDrawGUI(delta);

            ImGui::Render();

            global.projMatrix = global.camera.getProj();
            global.viewMatrix = global.camera.getView();

            renderer->render(glm::vec4(clearColor, 1.0));
        }
    }
}
}  // namespace vkr