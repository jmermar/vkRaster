#pragma once
#include "types.hpp"
namespace vkr {
struct SceneData {
    std::vector<ImageData> images;
    std::vector<Material> materials;
    std::vector<MeshData> meshes;
    struct InstanceInfo {
        size_t meshID;
        glm::mat4 transform;
    };

    std::vector<InstanceInfo> instances;
};

SceneData loadScene(const std::string& path);
};  // namespace vkr