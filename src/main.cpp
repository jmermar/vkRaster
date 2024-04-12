#pragma
#define SDL_MAIN_HANDLED

#include "Program.hpp"

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float a = 0;
    void onFrame(float deltaTime) override {}

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        renderer.meshHandler.clear();
        vkr::MeshData data;
        std::vector<vkr::Vertex> rect_vertices(4);

        rect_vertices[0].position = {0.5, -0.5, 0};
        rect_vertices[1].position = {0.5, 0.5, 0};
        rect_vertices[2].position = {-0.5, -0.5, 0};
        rect_vertices[3].position = {-0.5, 0.5, 0};

        rect_vertices[0].color = {0, 0, 0, 1};
        rect_vertices[1].color = {0.5, 0.5, 0.5, 1};
        rect_vertices[2].color = {1, 0, 0, 1};
        rect_vertices[3].color = {0, 1, 0, 1};

        rect_vertices[0].uv = {0, 0};
        rect_vertices[1].uv = {0, 0};
        rect_vertices[2].uv = {0, 0};
        rect_vertices[3].uv = {0, 0};

        std::vector<uint32_t> rect_indices((size_t)6);

        rect_indices[0] = 0;
        rect_indices[1] = 1;
        rect_indices[2] = 2;

        rect_indices[3] = 2;
        rect_indices[4] = 1;
        rect_indices[5] = 3;
        data.vertices = rect_vertices;
        data.indices = rect_indices;

        rect_vertices[0].position = {0, -1, 0};
        rect_vertices[1].position = {0, 0, 0};
        rect_vertices[2].position = {-1, -1, 0};
        rect_vertices[3].position = {-1, 0, 0};

        renderer.meshHandler.allocateMesh(data);

        data.vertices = rect_vertices;
        renderer.meshHandler.allocateMesh(data);

        renderer.meshHandler.build();
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}