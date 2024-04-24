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
    vkr::TransformData transform;

    void setScene(const std::string& name) {
        if (name == "car") {
            transform.scale = glm::vec3(25);
            loadScene("ToyCar.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        if (name == "duck") {
            transform.scale = glm::vec3(1);
            loadScene("Duck.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        if (name == "ducks") {
            transform.scale = glm::vec3(1);
            loadScene("lots_ducks.glb", transform);
            camera.position = {0.f, 0.f, 10.f};
            camera.lookAt(glm::vec3(0));
        }

        if (name == "sponza") {
            transform.scale = glm::vec3(1);

            loadScene("tmp/sponza/sponza.glb", transform);
            camera.position = {8.f, 1.f, 0.5f};
            camera.lookAt(glm::vec3(0));
        }
    }

    void onDrawGUI(float deltaTime) override {
        static bool showWindow = true;
        ImGui::SetNextWindowPos(ImVec2(16, 16));
        ImGui::Begin(
            "vkRaster", &showWindow,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);

        ImGui::Text("FPS: %f\n", 1.f / deltaTime);

        if (ImGui::Button("Car Scene")) {
            setScene("car");
        }

        if (ImGui::Button("Duck Scene")) {
            setScene("duck");
        }

        if (ImGui::Button("Ducks Scene")) {
            setScene("ducks");
        }

        if (ImGui::Button("Sponza Scene")) {
            setScene("sponza");
        }

        ImGui::InputFloat3("Camera Position", (float*)&camera.position, "%.3f",
                           ImGuiInputTextFlags_ReadOnly);

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

        camera.w = getWindowSize().w;
        camera.h = getWindowSize().h;

        proj = camera.getProj();
        view = camera.getView();

        clearColor = glm::vec3(0, 0, 0.4);

        if (isKeyDown(SDL_SCANCODE_F))
            cout << "FPS: " << 1.f / deltaTime << endl;
    }

   public:
    MyProgram() : vkr::Program({WIDTH, HEIGHT}, "vkRaster") {
        camera.znear = 0.1;
        camera.zfar = 1000;

        camera.w = WIDTH;
        camera.h = HEIGHT;

        setScene("ducks");
    }
};

int main() {
    MyProgram program;
    program.run();

    return 0;
}