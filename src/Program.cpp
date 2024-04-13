#include "Program.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>

#include "System.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "tinygltf.h"

void loadNode(vkr::Renderer& render, const tinygltf::Node& inputNode,
              const tinygltf::Model& input) {
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

    // Load node's children
    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            loadNode(render, input.nodes[inputNode.children[i]], input);
        }
    }

    // If the node contains mesh data, we load vertices and indices from the
    // buffers In glTF this is done via accessors and buffer views
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
        // Iterate through all primitives of this node's mesh
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            std::vector<uint32_t> indexBuffer;
            std::vector<vkr::Vertex> vertexBuffer;
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
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
                        input.accessors
                            [glTFPrimitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view =
                        input.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*>(
                        &(input.buffers[view.buffer]
                              .data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = accessor.count;
                }
                // Get buffer data for vertex normals
                if (glTFPrimitive.attributes.find("NORMAL") !=
                    glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor =
                        input.accessors[glTFPrimitive.attributes.find("NORMAL")
                                            ->second];
                    const tinygltf::BufferView& view =
                        input.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(
                        &(input.buffers[view.buffer]
                              .data[accessor.byteOffset + view.byteOffset]));
                }
                // Get buffer data for vertex texture coordinates
                // glTF supports multiple sets, we only load the first one
                if (glTFPrimitive.attributes.find("TEXCOORD_0") !=
                    glTFPrimitive.attributes.end()) {
                    const tinygltf::Accessor& accessor =
                        input.accessors[glTFPrimitive.attributes
                                            .find("TEXCOORD_0")
                                            ->second];
                    const tinygltf::BufferView& view =
                        input.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*>(
                        &(input.buffers[view.buffer]
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
                    input.accessors[glTFPrimitive.indices];
                const tinygltf::BufferView& bufferView =
                    input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer =
                    input.buffers[bufferView.buffer];

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
                        return;
                }
            }
            vkr::MeshData data;
            data.vertices = std::move(vertexBuffer);
            data.indices = std::move(indexBuffer);

            auto meshHandle = render.meshHandler.allocateMesh(data);
            render.unlitRenderer.addInstance(meshHandle, transform);
        }
    }
}

namespace vkr {
void Program::loadScene(const char* sceneName) {
    renderer.unlitRenderer.clear();
    renderer.meshHandler.clear();

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    std::string fullPath(std::string(RESPATH "models/") + sceneName);

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, fullPath.c_str());

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("ERR: %s\n", err.c_str());
    }
    if (!ret) {
        throw std::runtime_error("Cannot load scene " + std::string(sceneName));
    }

    // Read scene
    const tinygltf::Scene& scene = model.scenes[0];
    for (size_t i = 0; i < scene.nodes.size(); i++) {
        tinygltf::Node node = model.nodes[scene.nodes[i]];
        loadNode(renderer, node, model);
    }

    renderer.meshHandler.build();
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