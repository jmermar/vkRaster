#include "SceneLoader.hpp"

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "tinygltf.h"

namespace vkr {

struct LoadMeshData {
    struct PrimitiveData {
        size_t primitive;
        size_t material;
    };
    std::vector<PrimitiveData> primitives;
};

void loadNode(const glm::mat4& parentTransform, SceneData& scene,
              std::vector<LoadMeshData>& meshes,
              const tinygltf::Node& inputNode, const tinygltf::Model& input) {
    glm::mat4 transform(1);
    if (inputNode.translation.size() == 3) {
        transform = glm::translate(
            transform, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    }
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        transform *= glm::mat4(q);
    }
    if (inputNode.scale.size() == 3) {
        transform = glm::scale(
            transform, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    }
    if (inputNode.matrix.size() == 16) {
        transform = glm::make_mat4x4(inputNode.matrix.data());
    };

    transform = parentTransform * transform;

    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            loadNode(transform, scene, meshes,
                     input.nodes[inputNode.children[i]], input);
        }
    }

    if (inputNode.mesh > -1) {
        auto& mesh = meshes[inputNode.mesh];
        for (auto& primitive : mesh.primitives) {
            scene.instances.push_back(
                {primitive.primitive, transform, primitive.material});
        }
    }
}

SceneData loadScene(const std::string& path) {
    SceneData retScene;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    std::string extension;
    {
        std::filesystem::path filePath = path;
        extension = filePath.extension();
    }

    std::string fullPath(path);

    bool ret = false;
    if (extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, fullPath.c_str());
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, fullPath.c_str());
    }

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("ERR: %s\n", err.c_str());
    }
    if (!ret) {
        throw std::runtime_error("Cannot load scene " + fullPath);
    }

    // Load textures

    for (auto& image : model.images) {
        auto& imageData = retScene.textures.emplace_back();

        imageData.width = image.width;
        imageData.height = image.height;
        imageData.channels = image.component;
        auto size = imageData.width * imageData.height * imageData.channels;
        imageData.data.resize(size);
        memcpy(imageData.data.data(), image.image.data(), size);
    }

    // Load materials

#define GET_TEXTURE_IF_EXISTS(name)                        \
    ((material.values.find(name) != material.values.end()) \
         ? material.values[name].TextureIndex()            \
         : -1)

    for (auto& material : model.materials) {
        auto& materialData = retScene.materials.emplace_back();
        materialData.baseColortexture = -1;
        materialData.metallicRoughnessTexture = -1;
        materialData.normalTexture = -1;
        materialData.metallicRoughness = glm::vec4(0, 1, 0, 0);
        materialData.color = glm::vec4(1);

        materialData.baseColortexture =
            GET_TEXTURE_IF_EXISTS("baseColorTexture");
        materialData.metallicRoughnessTexture =
            GET_TEXTURE_IF_EXISTS("metallicRoughnessTexture");

        materialData.normalTexture = GET_TEXTURE_IF_EXISTS("normalTexture");

        if (material.values.find("baseColorFactor") != material.values.end()) {
            auto color = material.values["baseColorFactor"].ColorFactor();
            materialData.color.r = color[0];
            materialData.color.g = color[1];
            materialData.color.b = color[2];
        }
    }

#undef GET_TEXTURE_IF_EXISTS

    // Load meshes

    const auto& meshes = model.meshes;
    std::vector<LoadMeshData> meshesPrimitives;

    for (const auto& mesh : meshes) {
        meshesPrimitives.push_back({});
        auto& primitives =
            meshesPrimitives.at(meshesPrimitives.size() - 1).primitives;
        // Iterate through all primitives of this node's mesh
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            std::vector<uint32_t> indexBuffer;
            std::vector<vkr::Vertex> vertexBuffer;
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;
            // Vertices
            {
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                size_t vertexCount = 0;

                // Get buffer data for vertex positions
                if (glTFPrimitive.attributes.find("POSITION") !=
                    glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor =
                        model.accessors
                            [glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view =
                        model.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(
                        &(model.buffers[view.buffer]
                              .data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") !=
                    glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor =
                        model.accessors[glTFPrimitive.attributes.find("NORMAL")
                                            ->second];
                    const tinygltf::BufferView& view =
                        model.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(
                        &(model.buffers[view.buffer]
                              .data[accessor.byteOffset + view.byteOffset]));
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") !=
                    glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor =
                        model.accessors[glTFPrimitive.attributes
                                            .find("TEXCOORD_0")
                                            ->second];
                    const tinygltf::BufferView& view =
                        model.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(
                        &(model.buffers[view.buffer]
                              .data[accessor.byteOffset + view.byteOffset]));
                }

                // Append data to model's vertex buffer
                for (size_t v = 0; v < vertexCount; v++) {
                    vkr::Vertex vert{};
                    vert.position =
                        glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                    vert.normal = glm::normalize(glm::vec3(
                        normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3])
                                      : glm::vec3(0.0f)));
                    vert.uv = texCoordsBuffer
                                  ? glm::make_vec2(&texCoordsBuffer[v * 2])
                                  : glm::vec3(0.0f);
                    vert.color = glm::vec4(1.0f);
                    vertexBuffer.push_back(vert);
                }
            }
            // Indices
            {
                const tinygltf::Accessor& accessor =
                    model.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView =
                    model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer =
                    model.buffers[bufferView.buffer];

                indexCount += static_cast<uint32_t>(accessor.count);

                // glTF supports different component types of indices
                switch (accessor.componentType) {
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                        const uint32_t* buf = reinterpret_cast<const uint32_t*>(
                            &buffer.data[accessor.byteOffset +
                                         bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count;
                             index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(
                            &buffer.data[accessor.byteOffset +
                                         bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count;
                             index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(
                            &buffer.data[accessor.byteOffset +
                                         bufferView.byteOffset]);
                        for (size_t index = 0; index < accessor.count;
                             index++) {
                            indexBuffer.push_back(buf[index] + vertexStart);
                        }
                        break;
                    }
                    default:
                        continue;
                }
            }

            // Calculate tangent and bitangent

            for (size_t i = 0; i + 3 <= indexBuffer.size(); i += 3) {
                glm::vec3 pos1 = vertexBuffer[indexBuffer[i]].position;
                glm::vec3 pos2 = vertexBuffer[indexBuffer[i + 1]].position;
                glm::vec3 pos3 = vertexBuffer[indexBuffer[i + 2]].position;

                glm::vec2 uv1 = vertexBuffer[indexBuffer[i]].uv;
                glm::vec2 uv2 = vertexBuffer[indexBuffer[i + 1]].uv;
                glm::vec2 uv3 = vertexBuffer[indexBuffer[i + 2]].uv;

                glm::vec3 edge1 = pos2 - pos1;
                glm::vec3 edge2 = pos3 - pos1;

                glm::vec2 deltaUV1 = uv2 - uv1;
                glm::vec2 deltaUV2 = uv3 - uv1;

                glm::vec3 tangent, bitangent;
                float f =
                    1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                tangent = glm::normalize(tangent);

                bitangent.x =
                    f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y =
                    f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z =
                    f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                bitangent = glm::normalize(bitangent);

                vertexBuffer[indexBuffer[i]].tangent = tangent;
                vertexBuffer[indexBuffer[i + 1]].tangent = tangent;
                vertexBuffer[indexBuffer[i + 2]].tangent = tangent;

                vertexBuffer[indexBuffer[i]].bitangent = bitangent;
                vertexBuffer[indexBuffer[i + 1]].bitangent = bitangent;
                vertexBuffer[indexBuffer[i + 2]].bitangent = bitangent;
            }
            primitives.push_back(
                {retScene.meshes.size(), (size_t)mesh.primitives[i].material});
            retScene.meshes.push_back({});
            vkr::MeshData& data =
                retScene.meshes.at(retScene.meshes.size() - 1);
            data.vertices = std::move(vertexBuffer);
            data.indices = std::move(indexBuffer);
        }
    }

    // Read scene
    const tinygltf::Scene& scene = model.scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        tinygltf::Node node = model.nodes[scene.nodes[i]];
        loadNode(glm::mat4(1), retScene, meshesPrimitives, node, model);
    }

    std::cout << "Loaded scene from " << fullPath << ".\n";
    std::cout << "- " << retScene.meshes.size() << " meshes loaded.\n"
              << "- " << retScene.instances.size() << " nodes loaded.\n"
              << "- " << retScene.materials.size() << " materials loaded.\n"
              << "- " << retScene.textures.size() << " textures loaded."
              << std::endl;

    return retScene;
}
}  // namespace vkr