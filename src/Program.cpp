#include "Program.hpp"

#include <SceneLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

#include "System.hpp"
#include "rendering/SceneState.hpp"
namespace vkr {
void Program::loadScene(const char* sceneName, const TransformData& tdata) {
    auto scene = vkr::loadScene(sceneName);

    SceneState::get().clearScene();

    std::vector<vkr::TextureData*> textures;
    textures.reserve(scene.textures.size());
    for (auto& texture : scene.textures) {
        textures.push_back(SceneState::get().allocateTexture(
            texture.data.data(),
            {.width = texture.width, .height = texture.height, .depth = 1},
            VK_FORMAT_R8G8B8A8_UNORM, 0, vkr::GlobalBounds::SAMPLER_LINEAR));
    }

    for (auto& material : scene.materials) {
        SceneState::MaterialData data;
        data.texture = (material.texture >= 0)
                           ? textures[material.texture]->bindPoint
                           : (TextureBind)0;
        data.color = material.color;

        SceneState::get().getMaterials().add(data);
    }

    for (auto& mesh : scene.meshes) {
        SceneState::get().allocateMesh(mesh);
    }

    for (auto& ins : scene.instances) {
        auto t = tdata.getTransform() * ins.transform;

        SceneState::get().addInstance((vkr::BufferHandle)ins.meshID, t,
                                      ins.materialID);
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
            int w, h;
            SDL_GetWindowSize(system.getWindow(), &w, &h);
            size.w = w;
            size.h = h;

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