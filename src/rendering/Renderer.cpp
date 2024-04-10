#include "vkRaster/Renderer.hpp"

#include "RendererImp.hpp"
#include "System.hpp"

namespace vkr {
class MeshImp : public Mesh {
   public:
    GPUMesh data;
    RendererImp& imp;

    MeshImp(GPUMesh d, RendererImp& i) : data(d), imp(i) {}

    ~MeshImp() { imp.destroyMesh(data); }
};

Renderer::Renderer(System& system)
    : system(system),
      imp(std::make_unique<RendererImp>(system.getWindow(), system.getSize().w,
                                        system.getSize().h)) {}
void Renderer::drawFrame() { imp->render(); }
Renderer::~Renderer() {}

std::unique_ptr<Mesh> Renderer::createMesh(
    const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices) {
    return std::make_unique<MeshImp>(imp->uploadMesh(indices, vertices), *imp);
}
void Renderer::pushMesh(Mesh* m) {
    imp->temp_drawMeshes.push_back(((MeshImp*)m)->data);
}
}  // namespace vkr