#include "System.hpp"

#include <stdexcept>
namespace vkr {
System::System(const Size& size, const char* winName) {
    SDL_Init(SDL_INIT_VIDEO);
    this->size = size;
    window = SDL_CreateWindow(winName, size.w, size.h,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw std::runtime_error("Cannot create window, SDL Error: " +
                                 std::string(SDL_GetError()));
    }
}
System::~System() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void System::handleInput(bool& shouldQuit) {
    for (auto& [scan, key] : keysState) {
        if (key == KeyState::Pressed) key = KeyState::Hold;
    }

    SDL_Event ev;
    SDL_Scancode key;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                shouldQuit = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                key = ev.key.keysym.scancode;
                if (getKeyState(key) == KeyState::Released) {
                    keysState[key] = KeyState::Pressed;
                }
                break;

            case SDL_EVENT_KEY_UP:
                key = ev.key.keysym.scancode;
                keysState.erase(key);
                break;
        }
    }
}
System::KeyState System::getKeyState(SDL_Scancode key) {
    if (keysState.find(key) == keysState.end()) {
        return KeyState::Released;
    }
    return keysState[key];
}
}  // namespace vkr