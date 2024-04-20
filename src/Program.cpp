#include "Program.hpp"

#include <SceneLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "System.hpp"
#include "rendering/SceneState.hpp"

namespace vkr {
void Program::loadScene(const char* sceneName) {
    auto scene = vkr::loadScene(sceneName);

    uint8_t data[4] = {0, 255, 0, 0};

    for (auto& mesh : scene.meshes) {
        SceneState::get().allocateMesh(mesh);
    }

    for (auto& ins : scene.instances) {
        SceneState::get().addInstance((vkr::BufferHandle)ins.meshID,
                                      ins.transform);
    }
}
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

            SceneState::get().global.proj = proj;
            SceneState::get().global.view = view;

            renderer.render(glm::vec4(clearColor, 1.0));
        }
    }
}
}  // namespace vkr