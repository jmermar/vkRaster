#pragma once
#include <memory>

#include "System.hpp"
#include "rendering/Renderer.hpp"
#include "types.hpp"
namespace vkr {
class Program {
   private:
    Size size{};
    const char* winName{};
    System system{size, winName};
    Renderer renderer{system.getWindow(), size.w, size.h};

   protected:
    glm::mat4 proj, view;
    glm::vec3 clearColor;

    const Size& getWindowSize() { return size; }

    inline bool isKeyPressed(SDL_Scancode key) {
        return system.isKeyPressed(key);
    }

    inline bool isKeyDown(SDL_Scancode key) { return system.isKeyDown(key); }

    void loadScene(const char* scene, const TransformData& data);
    virtual void onFrame(float deltaTime) = 0;
    virtual void onDrawGUI(float deltaTime) = 0;

   public:
    Program(const Size& size, const char* name);
    ~Program();

    void run();
};
}  // namespace vkr