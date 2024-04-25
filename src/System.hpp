#pragma once
#include <SDL3/SDL.h>

#include <unordered_map>

#include "types.hpp"
namespace vkr {
class System {
   public:
    enum class KeyState { Pressed, Released, Hold };

   private:
    SDL_Window* window;
    Size size;

    std::unordered_map<SDL_Scancode, KeyState> keysState;

    glm::vec2 mouseDelta;
    bool captureMouse{};
    bool initCapture{};

   public:
    System(const Size& size, const char* winName);
    ~System();

    void handleInput(bool& shouldQuit);

    KeyState getKeyState(SDL_Scancode key);

    inline bool isKeyDown(SDL_Scancode key) {
        return getKeyState(key) != KeyState::Released;
    }
    inline bool isKeyPressed(SDL_Scancode key) {
        return getKeyState(key) == KeyState::Pressed;
    }

    inline void setCaptureMouse(bool capture) {
        initCapture = capture && !captureMouse;
        captureMouse = capture;
    }
    inline const glm::vec2& getMouseDelta() { return mouseDelta; }

    inline SDL_Window* getWindow() { return window; }
    inline const Size& getSize() { return size; }
};
}  // namespace vkr