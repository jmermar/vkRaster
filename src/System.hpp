#pragma once
#include <SDL3/SDL.h>

#include "types.hpp"

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