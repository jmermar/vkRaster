#define SDL_MAIN_HANDLED
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "vkRaster.hpp"

using namespace std;

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float walkSpeed = 5.f;
    vkr::CameraData camera;
    vkr::TransformData transform;
    void onFrame(float deltaTime) override {
        if (isKeyDown(SDL_SCANCODE_W)) {
            camera.position += camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_S)) {
            camera.position -= camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_LEFT)) {
            camera.rotateX(45.f * deltaTime);
        }

        if (isKeyDown(SDL_SCANCODE_RIGHT)) {
            camera.rotateX(-45.f * deltaTime);
        }

        if (isKeyDown(SDL_SCANCODE_UP)) {
            camera.rotateY(45.f * deltaTime);
        }

        if (isKeyDown(SDL_SCANCODE_DOWN)) {
            camera.rotateY(-45.f * deltaTime);
        }

        if (isKeyPressed(SDL_SCANCODE_1)) {
            transform.scale = glm::vec3(25);
            loadScene("ToyCar.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        if (isKeyPressed(SDL_SCANCODE_2)) {
            transform.scale = glm::vec3(0.01);
            loadScene("Duck.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        if (isKeyPressed(SDL_SCANCODE_3)) {
            transform.scale = glm::vec3(1);

            loadScene("tmp/sponza/sponza.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        proj = camera.getProj();
        view = camera.getView();

        clearColor = glm::vec3(0, 0, 0.4);

        if (isKeyDown(SDL_SCANCODE_F))
            cout << "FPS: " << 1.f / deltaTime << endl;
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        transform.scale = glm::vec3(0.01);
        loadScene("Duck.glb", transform);
        camera.znear = 0.1;
        camera.zfar = 1000;
        camera.position = {0.f, 0.f, 10.f};
        camera.lookAt(glm::vec3(0));

        camera.w = WIDTH;
        camera.h = HEIGHT;
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}