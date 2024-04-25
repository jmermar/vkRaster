#include "System.hpp"

#include <backends/imgui_impl_sdl3.h>

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
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    glm::vec2 center(w / 2.f, h / 2.f);
    if (captureMouse) {
        SDL_SetRelativeMouseMode(true);
        if (!initCapture) {
            glm::vec2 mousePos;
            SDL_GetMouseState(&mousePos.x, &mousePos.y);

            mouseDelta = center - mousePos;
        }

        SDL_WarpMouseInWindow(window, center.x, center.y);
    } else {
        SDL_SetRelativeMouseMode(false);
        mouseDelta = {};
    }
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
        ImGui_ImplSDL3_ProcessEvent(&ev);
    }
}
System::KeyState System::getKeyState(SDL_Scancode key) {
    if (keysState.find(key) == keysState.end()) {
        return KeyState::Released;
    }
    return keysState[key];
}
}  // namespace vkr