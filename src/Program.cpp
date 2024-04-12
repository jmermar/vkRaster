#include "Program.hpp"

#include "System.hpp"
namespace vkr {
Program::Program(const Size& size, const char* name)
    : size(size), winName(name) {}
Program::~Program() {}
void Program::run() {
    bool shouldQuit = false;

    Uint32 ticks = SDL_GetTicks();

    while (!shouldQuit) {
        float delta = (SDL_GetTicks() - ticks) / 1000.f;
        ticks = SDL_GetTicks();
        system.handleInput(shouldQuit);

        onFrame(delta);

        renderer.render();
    }
}
}  // namespace vkr