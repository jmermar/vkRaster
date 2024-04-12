#pragma once
#include "vkRaster/vkRaster.hpp"

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float a = 0;
    vkr::Mesh mesh;
    void onFrame(float deltaTime) override {
        a += deltaTime;
        if (a > 1) a = 0;
        std::vector<vkr::Vertex> rect_vertices(4);

        rect_vertices[0].position = {0.5, -0.5, 0};
        rect_vertices[1].position = {0.5, 0.5, 0};
        rect_vertices[2].position = {-0.5, -0.5, 0};
        rect_vertices[3].position = {-0.5, 0.5, 0};

        rect_vertices[0].color = {1, a, 0, 1};
        rect_vertices[1].color = {0.5, a,  0.5, 1};
        rect_vertices[2].color = {1, 0, 0.5 * a, 1};
        rect_vertices[3].color = {0.5 + a * 0.5, a, 0, a};

        std::vector<unsigned int> rect_indices;
        rect_indices.resize(6);

        rect_indices[0] = 0;
        rect_indices[1] = 1;
        rect_indices[2] = 2;

        rect_indices[3] = 2;
        rect_indices[4] = 1;
        rect_indices[5] = 3;

        mesh = renderer.createMesh(rect_indices, rect_vertices);
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {}
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}