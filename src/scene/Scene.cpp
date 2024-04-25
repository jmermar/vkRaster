#include "Scene.hpp"

#include <SceneLoader.hpp>

namespace vkr {
Scene::Scene() {}
Scene::~Scene() {}
ModelHandle Scene::loadModel(const std::string& path) {
    if (modelsLoaded.contains(path)) {
        return modelsLoaded[path];
    }
    auto scene = vkr::loadScene(path);

    ModelStorage model{};

    std::vector<TextureHandle> textures;
    textures.reserve(scene.textures.size());

    for (auto& texture : scene.textures) {
        textures.push_back(allocateTexture(
            texture.data.data(), {.w = texture.width, .h = texture.height},
            TEXTURE_FORMAT_RGBA32, 0, GlobalBounds::SAMPLER_LINEAR));
    }

    for (auto& material : scene.materials) {
        SceneState::MaterialData data;
        auto tHandle = textures[material.texture];
        data.texture = (material.texture >= 0)
                           ? this->textures.get(tHandle)->bindPoint
                           : (TextureBind)0;
        data.color = material.color;

        model.materials.push_back(SceneState::get().getMaterials().add(data));
    }

    for (auto& mesh : scene.meshes) {
        model.meshes.push_back(SceneState::get().allocateMesh(mesh));
    }

    for (auto& ins : scene.instances) {
        ModelStorage::Instance insData;
        insData.mesh = ins.meshID;
        insData.material = ins.materialID;
        insData.transform = ins.transform;
        model.instances.push_back(insData);
    }
    auto handle = models.add(model);
    modelsLoaded[path] = handle;
    return handle;
}
TextureHandle Scene::getTexture(const std::string& path) {}
TextureHandle Scene::allocateTexture(void* data, Size size,
                                     TextureFormat format,
                                     VkImageUsageFlags usage,
                                     GlobalBounds::SamplerType samplerType) {
    VkExtent3D extent{.width = size.w, .height = size.h, .depth = 1};
    VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    if (format == TEXTURE_FORMAT_FLOAT32) {
        vkFormat = VK_FORMAT_D32_SFLOAT;
    }

    auto tex = sceneState.allocateTexture(data, extent, vkFormat, 0,
                                          vkr::GlobalBounds::SAMPLER_LINEAR);

    auto handle = textures.add(tex);
    return handle;
}
void Scene::addInstance(ModelHandle model, const glm::mat4& transform) {
    auto& modelData = *models.get(model);
    for (auto& ins : modelData.instances) {
        sceneState.addInstance(modelData.meshes[ins.mesh],
                               transform * ins.transform,
                               modelData.materials[ins.material]);
    }
}
void Scene::clearTextures() {
    auto data = textures.getData();
    for (int i = 0; i < textures.getSize(); i++) {
        sceneState.freeTexture(data[i]);
    }

    textures.clear();
}
void Scene::clearModels() {
    models.clear();
    modelsLoaded.clear();
}
}  // namespace vkr