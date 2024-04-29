#define SDL_MAIN_HANDLED
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "rendering/GlobalRenderData.hpp"
#include "rendering/SceneState.hpp"
#include "vkRaster.hpp"
using namespace std;

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

class MyProgram : public vkr::Program {
   protected:
    float walkSpeed = 5.f;
    vkr::CameraData camera;
    bool onMenu = false;

    std::vector<vkr::Light> lights;

    vkr::Light cameraLight;

    float intensity = 10;
    float radius = 0;

    void init(bool sponza = false) {
        scene.clear();
        lights.clear();
        camera.position = {0, 1, 0};
        camera.target = {0, 0, 1};

        if (sponza) {
            vkr::TransformData transform;
            scene.addInstance(scene.loadModel("tmp/sponza/sponza.glb"),
                              transform.getTransform());

            for (int y = -10; y < 10; y++) {
                for (int x = -10; x < 10; x++) {
                    for (int z = -10; z < 10; z++) {
                        lights.push_back(scene.addLight(
                            glm::vec3(x, y + 0.5, z) * 4.f, 4, 6));
                    }
                }
            }
        } else {
            vkr::TransformData transform;
            transform.position.z = 5;
            scene.addInstance(scene.loadModel("Duck.glb"),
                              transform.getTransform());
        }
    }

    void onDrawGUI(float deltaTime) override {
        bool showMenu = onMenu;
        bool isTrue = true;
        ImGui::SetNextWindowPos(ImVec2(16, 16));
        ImGui::Begin(
            "vkRaster", &isTrue,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);

        ImGui::Text("FPS: %f\n", 1.f / deltaTime);

        ImGui::InputFloat3("Camera Position", (float*)&camera.position, "%.3f",
                           ImGuiInputTextFlags_ReadOnly);

        ImGui::End();

        if (onMenu) {
            ImGui::Begin("Menu", &showMenu, 0);

            ImGui::LabelText("Scenes", "Scenes");
            if (ImGui::Button("duck")) {
                init();
            }

            if (ImGui::Button("Sponza")) {
                init(true);
            }

            ImGui::LabelText("Light", "Scenes");
            if (ImGui::Button("Add Light")) {
                lights.push_back(
                    scene.addLight(camera.position, radius, intensity));
            }

            ImGui::SliderFloat("Intensity", &intensity, 0, 20);
            ImGui::SliderFloat("Distance", &radius, 0, 20);

            ImGui::LabelText("Add Objects", "Scenes");

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

            if (ImGui::Button("Add Damaged Helmet")) {
                vkr::TransformData transform;
                transform.position = camera.position;
                scene.addInstance(scene.loadModel("DamagedHelmet.glb"),
                                  transform.getTransform());
            }

            if (ImGui::Button("Add Sponza")) {
                vkr::TransformData transform;
                transform.position = camera.position;
                scene.addInstance(scene.loadModel("tmp/sponza/sponza.glb"),
                                  transform.getTransform());
            }

            vkr::GlobalRenderData::get().frustumCulling = false;
            static bool frustumEnabled = true;
            ImGui::Checkbox("Frustum culling", &frustumEnabled);

            if (frustumEnabled)
                vkr::GlobalRenderData::get().frustumCulling = true;
            ImGui::End();
        }
    }

    void onFrame(float deltaTime) override {
        if (isKeyDown(SDL_SCANCODE_W)) {
            camera.position += camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_S)) {
            camera.position -= camera.target * deltaTime * walkSpeed;
        }

        float sensitivity = 180.f;

        camera.rotateX(45.f * getMouseX() * sensitivity * deltaTime);
        camera.rotateY(45.f * getMouseY() * sensitivity * deltaTime);

        if (isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            onMenu = !onMenu;
        }

        setCaptureMouse(!onMenu);

        cameraLight.setPosition(camera.position);
        cameraLight.setIntensity(intensity);
        cameraLight.setRadius(radius);

        camera.w = getWindowSize().w;
        camera.h = getWindowSize().h;

        proj = camera.getProj();
        view = camera.getView();

        vkr::GlobalRenderData::get().camera = camera;

        clearColor = glm::vec3(0, 0, 0);
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        camera.znear = 0.1;
        camera.zfar = 1000;

        camera.w = WIDTH;
        camera.h = HEIGHT;

        cameraLight = scene.addLight(camera.position, radius, intensity);

        init();
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}