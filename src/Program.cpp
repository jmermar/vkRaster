#include "Program.hpp"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

#include "System.hpp"
#include "rendering/SceneState.hpp"
namespace vkr {
Program::Program(const Size& size, const char* name)
    : size(size), winName(name) {}
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
            size.w = w;
            size.h = h;

            ticks = SDL_GetTicks();
            system.handleInput(shouldQuit);

            onFrame(delta);

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            onDrawGUI(delta);

            ImGui::Render();

            SceneState::get().global.proj = proj;
            SceneState::get().global.view = view;

            renderer.render(glm::vec4(clearColor, 1.0));
        }
    }
}
}  // namespace vkr