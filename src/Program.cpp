#include "Program.hpp"

#include <SceneLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "System.hpp"

namespace vkr {
void Program::loadScene(const char* sceneName) {
    renderer.unlitRenderer.clear();
    renderer.meshHandler.clear();

    auto scene = vkr::loadScene(sceneName);

    std::vector<MeshHandle*> meshes;
    meshes.reserve(scene.meshes.size());
    for (const auto& mesh : scene.meshes) {
        meshes.push_back(renderer.meshHandler.allocateMesh(mesh));
    }
    renderer.meshHandler.build();

    for (const auto& [meshID, transform] : scene.instances) {
        renderer.unlitRenderer.addInstance(meshes[meshID], transform);
    }

    renderer.unlitRenderer.build();
}
Program::Program(const Size& size, const char* name)
    : size(size), winName(name) {
    renderer.unlitRenderer.clear();
    renderer.meshHandler.clear();
}
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

            renderer.renderData.clearColor = clearColor;
            renderer.renderData.project = proj;
            renderer.renderData.view = view;
            renderer.renderData.clearColor = clearColor;

            renderer.render();
        }
    }
}
}  // namespace vkr