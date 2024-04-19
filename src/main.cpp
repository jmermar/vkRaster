#define SDL_MAIN_HANDLED
#include <glm/gtc/matrix_transform.hpp>

#include "vkRaster.hpp"

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float a = 0;
    void onFrame(float deltaTime) override {
        a += deltaTime;

        proj =
            glm::perspective(45.f, (float)WIDTH / (float)HEIGHT, 0.05f, 1000.f);
        view = glm::lookAt(glm::vec3(10, 10, 10), glm::vec3(0),
                           glm::vec3(0, 1, 0));
        clearColor = glm::vec3(0, 0, 0.4);
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        loadScene("test2.glb");
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}