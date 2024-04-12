#pragma
#define SDL_MAIN_HANDLED

#include <glm/gtc/matrix_transform.hpp>

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

        rect_vertices[0].position = {1, -1, 0};
        rect_vertices[1].position = {1, 1, 0};
        rect_vertices[2].position = {-1, -1, 0};
        rect_vertices[3].position = {-1, 1, 0};

        rect_vertices[0].color = {0, 0, 0, 1};
        rect_vertices[1].color = {1, 0.5, 0.5, 1};
        rect_vertices[2].color = {1, 0, 0, 1};
        rect_vertices[3].color = {0, 1, 0, 1};

        rect_vertices[0].uv = {0, 0};
        rect_vertices[1].uv = {0, 0};
        rect_vertices[2].uv = {0, 0};
        rect_vertices[3].uv = {0, 0};

        data.vertices = rect_vertices;
        data.indices.resize(6);
        data.indices[0] = 0;
        data.indices[1] = 1;
        data.indices[2] = 2;

        data.indices[3] = 2;
        data.indices[4] = 1;
        data.indices[5] = 3;

        auto mesh = renderer.meshHandler.allocateMesh(data);

        auto data2 = data;

        data2.vertices[0].position = {0.1, -0.1, 0};
        data2.vertices[1].position = {0.1, 0.5, 0};
        data2.vertices[2].position = {-0.1, -0.1, 0};
        data2.vertices[3].position = {-0.1, 0.1, 0};

        auto mesh2 = renderer.meshHandler.allocateMesh(data2);

        renderer.meshHandler.build();
        renderer.unlitRenderer.clear();
        renderer.unlitRenderer.addInstance(
            mesh, glm::translate(glm::mat4(1.f), glm::vec3(-0.5f, 0.f, 0.f)) *
                      glm::scale(glm::mat4(1.f), glm::vec3(0.25f)));
        renderer.unlitRenderer.addInstance(
            mesh2, glm::translate(glm::mat4(1.f), glm::vec3(0.5f, 0.f, 0.f)) *
                       glm::scale(glm::mat4(1.f), glm::vec3(2.f)));
        renderer.unlitRenderer.build();
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}