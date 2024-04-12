#pragma once
#include <memory>
#include <string>
#include <vector>

#include "types.hpp"
namespace vkr {
class RendererImp;
class System;
class Program;
class Renderer;
struct GPUMesh;

class Mesh {
   private:
    GPUMesh* data{};
    RendererImp* imp{};
    std::string name{};

    void free();

    Mesh(GPUMesh* mesh, RendererImp* imp, const std::string& name)
        : data(mesh), imp(imp), name(name) {}

   public:
    Mesh() = default;
    ~Mesh() { free(); }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& o) {
        data = o.data;
        imp = o.imp;
        name = o.name;
        o.data = 0;
    }

    Mesh& operator=(Mesh&& o) {
        free();
        data = o.data;
        imp = o.imp;
        name = o.name;
        o.data = 0;
        return *this;
    }

    const std::string& getName() { return name; }

    friend class Renderer;
};

class Renderer {
   private:
    Renderer(System& system);
    System& system;
    std::unique_ptr<RendererImp> imp;

    void drawFrame();

   public:
    ~Renderer();

    Mesh createMesh(const std::vector<unsigned int>& indices,
                    const std::vector<Vertex>& vertices);

    friend class Program;
};
}  // namespace vkr