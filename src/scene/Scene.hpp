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
using LightPointHandle = PackedArray<LightPoint>::Handle;
class Light {
   private:
    PackedArray<LightPoint>* data{};
    LightPointHandle handle{-1};
    Light(LightPointHandle handle, PackedArray<LightPoint>* data)
        : handle(handle), data(data) {}

    inline void destroy() {
        if (data && handle >= 0) {
            data->remove(handle);
            data = 0;
            handle = -1;
        }
    }

    friend class Scene;

   public:
    ~Light() { destroy(); }
    Light() = default;
    Light(const Light& other) {
        handle = other.data->add(*other.data->get(other.handle));
        data = other.data;
    }

    Light& operator=(const Light& other) {
        destroy();
        handle = other.data->add(*other.data->get(other.handle));
        data = other.data;

        return *this;
    }

    Light(Light&& other) {
        handle = other.handle;
        data = other.data;
        other.data = 0;
        other.handle = -1;
    }

    Light& operator=(Light&& other) {
        destroy();
        handle = other.handle;
        data = other.data;
        other.data = 0;
        other.handle = -1;

        return *this;
    }

    const glm::vec3& getPosition() { return data->get(handle)->pos; }

    void setPosition(const glm::vec3& position) {
        data->get(handle)->pos = position;
    }

    void setIntensity(float intensity) {
        data->get(handle)->intensity = intensity;
    }

    void setRadius(float radius) { data->get(handle)->radius = radius; }
};

using ModelHandle = PackedArray<ModelStorage>::Handle;
class Scene {
    friend class Program;

   private:
    using TextureDataArray = PackedArray<TextureData>;
    using ModelArray = PackedArray<ModelStorage>;

    TextureDataArray textures;
    ModelArray models;
    PackedArray<LightPoint> lights;

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

    void update();

    void addInstance(ModelHandle model, const glm::mat4& transform);
    Light addLight(const glm::vec3& position, float radius, float intensity,
                   const glm::vec3& color = {1, 1, 1});

    inline void clear() {
        clearTextures();
        clearModels();
        sceneState.clearScene();
    }
};
}  // namespace vkr