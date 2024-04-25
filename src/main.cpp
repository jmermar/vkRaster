#define SDL_MAIN_HANDLED
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "rendering/SceneState.hpp"
#include "vkRaster.hpp"
using namespace std;

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float walkSpeed = 5.f;
    vkr::CameraData camera;
    bool onMenu = false;

    void init(bool sponza = false) {
        scene.clear();
        camera.position = {};
        camera.target = {0, 0, 1};

        if (sponza) {
            vkr::TransformData transform;
            scene.addInstance(scene.loadModel("tmp/sponza/sponza.glb"),
                              transform.getTransform());
        } else {
            vkr::TransformData transform;
            scene.addInstance(scene.loadModel("Duck.glb"),
                              transform.getTransform());
        }
    }

    void onDrawGUI(float deltaTime) override {
        static bool showWindow = true;
        ImGui::SetNextWindowPos(ImVec2(16, 16));
        ImGui::Begin(
            "vkRaster", &showWindow,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);

        ImGui::Text("FPS: %f\n", 1.f / deltaTime);

        if (ImGui::Button("Restart")) {
            init();
        }

        if (ImGui::Button("Add Duck")) {
            vkr::TransformData transform;
            transform.position = camera.position;
            scene.addInstance(scene.loadModel("Duck.glb"),
                              transform.getTransform());
        }

        if (ImGui::Button("Add Car")) {
            vkr::TransformData transform;
            transform.scale = glm::vec3(45);
            transform.position = camera.position;
            scene.addInstance(scene.loadModel("ToyCar.glb"),
                              transform.getTransform());
        }

        ImGui::InputFloat3("Camera Position", (float*)&camera.position, "%.3f",
                           ImGuiInputTextFlags_ReadOnly);

        if (ImGui::Button("Add Sponza")) {
            vkr::TransformData transform;
            transform.position = camera.position;
            scene.addInstance(scene.loadModel("tmp/sponza/sponza.glb"),
                              transform.getTransform());
        }

        vkr::SceneState::get().global.culling = 0;
        static bool frustumEnabled = true;
        ImGui::Checkbox("Frustum culling", &frustumEnabled);

        if (frustumEnabled)
            vkr::SceneState::get().global.culling |= vkr::CULLING_TYPE_FRUSTUM;

        ImGui::ShowDemoWindow();

        ImGui::End();
    }

    void onFrame(float deltaTime) override {
        if (isKeyDown(SDL_SCANCODE_W)) {
            camera.position += camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_S)) {
            camera.position -= camera.target * deltaTime * walkSpeed;
        }

        camera.rotateX(45.f * getMouseX());
        camera.rotateY(45.f * getMouseY());

        if (isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            onMenu = !onMenu;
        }

        setCaptureMouse(!onMenu);

        camera.w = getWindowSize().w;
        camera.h = getWindowSize().h;

        proj = camera.getProj();
        view = camera.getView();

        clearColor = glm::vec3(0, 0, 0.4);
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        camera.znear = 0.1;
        camera.zfar = 1000;

        camera.w = WIDTH;
        camera.h = HEIGHT;

        init();
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}