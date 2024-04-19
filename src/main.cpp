#define SDL_MAIN_HANDLED
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/SceneState.hpp"
#include "vkRaster.hpp"

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float a = 0;
    void onFrame(float deltaTime) override {
        a += deltaTime;

        a = 0;
        proj =
            glm::perspective(45.f, (float)WIDTH / (float)HEIGHT, 0.05f, 1000.f);
        view = glm::lookAt(glm::vec3(100, 100, 100), glm::vec3(0),
                           glm::vec3(0, 1, 0));
        clearColor = glm::vec3(0, 0, 0.4);
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        loadScene("test.glb");

        vkr::SceneState::get().clearMeshes();
        vkr::SceneState::get().clearInstances();

        vkr::MeshData data;
        data.vertices.resize(3);
        data.indices.resize(3);

        data.vertices[0].position = glm::vec3(0, -1, 0);
        data.vertices[1].position = glm::vec3(-1, 1, 0);
        data.vertices[2].position = glm::vec3(1, 1, 0);

        data.indices[0] = 0;
        data.indices[1] = 1;
        data.indices[2] = 2;

        auto mesh = vkr::SceneState::get().allocateMesh(data);
        auto mesh2 = vkr::SceneState::get().allocateMesh(data);
        vkr::SceneState::get().addInstance(mesh, glm::mat4(1));
        vkr::SceneState::get().addInstance(mesh, glm::mat4(1));
        vkr::SceneState::get().addInstance(mesh2, glm::mat4(1));
        vkr::SceneState::get().addInstance(mesh2, glm::mat4(1));
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}