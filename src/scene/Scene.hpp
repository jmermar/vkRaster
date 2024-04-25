#pragma once
#include <unordered_map>

#include "../PackedArray.inl"
#include "../rendering/SceneState.hpp"
namespace vkr {
using TextureHandle = PackedArray<TextureData>::Handle;
struct ModelStorage {
    std::vector<BufferHandle> meshes;
    std::vector<MaterialHandle> materials;

    struct Instance {
        uint32_t mesh;
        uint32_t material;
        glm::mat4 transform;
    };
    std::vector<Instance> instances;
};

using ModelHandle = PackedArray<ModelStorage>::Handle;
class Scene {
    friend class Program;

   private:
    using TextureDataArray = PackedArray<TextureData>;
    using ModelArray = PackedArray<ModelStorage>;

    TextureDataArray textures;
    ModelArray models;

    SceneState& sceneState{SceneState::get()};

    std::unordered_map<std::string, ModelHandle> modelsLoaded;

    Scene();
    void clearTextures();
    void clearModels();

   public:
    ~Scene();

    ModelHandle loadModel(const std::string& path);

    TextureHandle getTexture(const std::string& path);
    TextureHandle allocateTexture(void* data, Size size, TextureFormat format,
                                  VkImageUsageFlags usage,
                                  GlobalBounds::SamplerType samplerType);

    void addInstance(ModelHandle model, const glm::mat4& transform);

    inline void clear() {
        clearTextures();
        clearModels();
        sceneState.clearScene();
    }
};
}  // namespace vkr