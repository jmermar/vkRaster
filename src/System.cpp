#include "System.hpp"
#include <stdexcept>
namespace vkr {
System::System(const Size& size, const char* winName) {
    SDL_Init(SDL_INIT_VIDEO);
    this->size = size;
    window = SDL_CreateWindow(winName, size.w, size.h,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        throw std::runtime_error("Cannot create window, SDL Error: " + std::string(SDL_GetError()));
    }
}
System::~System() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void System::handleInput(bool& shouldQuit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_EVENT_QUIT:
                shouldQuit = true;
                break;
        }
    }
}
}  // namespace vkr