#include "Program.hpp"

#include <SceneLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "System.hpp"

namespace vkr {
void Program::loadScene(const char* sceneName) {}
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
            ticks = SDL_GetTicks();
            system.handleInput(shouldQuit);

            onFrame(delta);

            renderer.render(glm::vec4(clearColor, 1.0));
        }
    }
}
}  // namespace vkr