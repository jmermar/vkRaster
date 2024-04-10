#include "System.hpp"

namespace vkr {
System::System(const Size& size, const char* winName) {
    SDL_Init(SDL_INIT_VIDEO);
    this->size = size;
    window = SDL_CreateWindow(winName, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, size.w, size.h,
                              SDL_WINDOW_VULKAN);
}
System::~System() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void System::handleInput(bool& shouldQuit) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                shouldQuit = true;
                break;
        }
    }
}
}  // namespace vkr