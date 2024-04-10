#pragma once
#include <memory>
#include <vector>

#include "types.hpp"
namespace vkr {
class RendererImp;
class System;
class Program;
class Renderer;

class Mesh {
   public:
    virtual ~Mesh() {}
};

class Renderer {
   private:
    Renderer(System& system);
    System& system;
    std::unique_ptr<RendererImp> imp;

    void drawFrame();

   public:
    ~Renderer();

    std::unique_ptr<Mesh> createMesh(const std::vector<unsigned int>& indices,
                                     const std::vector<Vertex>& vertices);

    void pushMesh(Mesh* m);

    friend class Program;
};
}  // namespace vkr