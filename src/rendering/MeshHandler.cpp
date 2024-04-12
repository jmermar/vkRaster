#include "MeshHandler.hpp"

#include "Renderer.hpp"
namespace vkr {
MeshHandler::MeshHandler(Renderer& render) : render(render) {}
MeshHandler::~MeshHandler() { render.destroyMesh(buffer); }
MeshHandle* MeshHandler::allocateMesh(const MeshData& data) {
    MeshHandle handle;
    handle.firstVertex = this->data.vertices.size();
    handle.firstIndex = this->data.indices.size();
    handle.numIndex = data.indices.size();
    handle.numVertex = data.vertices.size();

    this->data.vertices.insert(this->data.vertices.end(), data.vertices.begin(),
                               data.vertices.end());
    this->data.indices.insert(this->data.indices.end(), data.indices.begin(),
                              data.indices.end());
    meshes.push_back(handle);

    return &meshes[meshes.size() - 1];
}
void MeshHandler::clear() {
    meshes.clear();
    data.vertices.clear();
    data.indices.clear();

    render.destroyMesh(buffer);
}
void MeshHandler::build() { buffer = render.uploadMesh(data); }
}  // namespace vkr