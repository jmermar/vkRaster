#include "vkRaster/Renderer.hpp"

#include "RendererImp.hpp"
#include "System.hpp"

namespace vkr {

Renderer::Renderer(System& system)
    : system(system),
      imp(std::make_unique<RendererImp>(system.getWindow(), system.getSize().w,
                                        system.getSize().h)) {}
void Renderer::drawFrame() { imp->render(); }
Renderer::~Renderer() {}

Mesh Renderer::createMesh(
    const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices) {
    static int i = 0;
    const auto name = "Mesh" + std::to_string(i++);
    return Mesh(imp->uploadMesh(name, indices, vertices), imp.get(), name);
}
void Mesh::free() {
    if (data) {
        imp->destroyMesh(name);
        data = 0;
    }
}
}  // namespace vkr