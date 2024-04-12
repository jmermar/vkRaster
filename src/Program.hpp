#pragma once
#include <memory>

#include "Rendering/Renderer.hpp"
#include "System.hpp"
#include "types.hpp"
namespace vkr {
class Program {
   private:
    Size size{};
    const char* winName{};
    System system{size, winName};

   protected:
    Renderer renderer{system.getWindow(), size.w, size.h};
    virtual void onFrame(float deltaTime) = 0;

   public:
    Program(const Size& size, const char* name);
    ~Program();

    void run();
};
}  // namespace vkr