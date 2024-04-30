#define SDL_MAIN_HANDLED
#include <imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>

#include "rendering/GlobalRenderData.hpp"
#include "rendering/SceneState.hpp"
#include "vkRaster.hpp"
using namespace std;

constexpr uint32_t WIDTH = 1920, HEIGHT = 1080;

inline float randFloat() { return (rand() % 1000) / 999.f; }

class MyProgram : public vkr::Program {
   protected:
    float walkSpeed = 5.f;
    vkr::CameraData camera;
    bool onMenu = false;

    std::vector<vkr::Light> lights;

    vkr::Light cameraLight;

    std::string sceneName;

    int numLights = 120;
    float maxLightDist = 10;

    void generateLights() {
        lights.clear();
        for (int i = 0; i < numLights; i++) {
            float x = randFloat();
            float y = randFloat();
            float z = randFloat();

            float r = 0.25 + 0.75 * randFloat();
            float g = 0.25 + 0.75 * randFloat();
            float b = 0.25 + 0.75 * randFloat();
            glm::vec3 pos =
                (glm::vec3(x, y, z) * 2.f - glm::vec3(1)) * maxLightDist;

            float radius = 2.5 + randFloat() * 2;
            lights.push_back(scene.addLight(pos, radius, 2, {r, g, b}));
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

            vkr::GlobalRenderData::get().frustumCulling = false;
            static bool frustumEnabled = true;
            ImGui::Checkbox("Frustum culling", &frustumEnabled);

            if (frustumEnabled)
                vkr::GlobalRenderData::get().frustumCulling = true;

            if (ImGui::InputInt("Number of lights", &numLights, 100, 1000)) {
                generateLights();
            }

            if (ImGui::InputFloat("Light Max Distance", &maxLightDist, 100,
                                  1000)) {
                generateLights();
            }
            ImGui::End();
        }
    }

    void onFrame(float deltaTime) override {
        for (int i = 0; i < numLights; i++) {
            auto& light = lights[i].getPosition();
            auto n = light * (1.f / glm::length(light));
            auto c = glm::vec3(1, 0, 0);

            auto dir = glm::cross(n, c);

            lights[i].setPosition(light + dir * 2.f * deltaTime);
        }

        if (isKeyDown(SDL_SCANCODE_W)) {
            camera.position += camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_S)) {
            camera.position -= camera.target * deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_A)) {
            camera.position += glm::cross(camera.target, glm::vec3(0, -1, 0)) *
                               deltaTime * walkSpeed;
        }

        if (isKeyDown(SDL_SCANCODE_D)) {
            camera.position -= glm::cross(camera.target, glm::vec3(0, 1, 0)) *
                               deltaTime * walkSpeed;
        }

        float sensitivity = 180.f;

        camera.rotateX(45.f * getMouseX() * sensitivity * deltaTime);
        camera.rotateY(45.f * getMouseY() * sensitivity * deltaTime);

        if (isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            onMenu = !onMenu;
        }

        setCaptureMouse(!onMenu);

        camera.w = getWindowSize().w;
        camera.h = getWindowSize().h;

        proj = camera.getProj();
        view = camera.getView();

        vkr::GlobalRenderData::get().camera = camera;

        clearColor = glm::vec3(0, 0, 0);
    }

   public:
    MyProgram(const char* sceneName)
        : vkr::Program({WIDTH, HEIGHT}, "vkRaster"), sceneName(sceneName) {
        camera.znear = 0.1;
        camera.zfar = 1000;

        camera.w = WIDTH;
        camera.h = HEIGHT;

        scene.clear();
        lights.clear();
        camera.position = {0, 1, 0};
        camera.target = {0, 0, 1};
        vkr::TransformData transform;
        scene.addInstance(scene.loadModel(sceneName), transform.getTransform());
        numLights = 1000;
        generateLights();
    }
};

int main(int argc, char** args) {
    if (argc < 2) {
        cout << "Usage: " << args[0] << " <model path>" << endl;
        return 0;
    }
    MyProgram program(args[1]);
    program.run();

    return 0;
}