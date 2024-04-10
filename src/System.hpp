#pragma once
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "vkRaster/types.hpp"

namespace vkr {
class System {
   private:
    SDL_Window* window;
    Size size;

   public:
    System(const Size& size, const char* winName);
    ~System();

    void handleInput(bool& shouldQuit);

    inline SDL_Window* getWindow() { return window; }
    inline const Size& getSize() { return size; }
};
}  // namespace vkr