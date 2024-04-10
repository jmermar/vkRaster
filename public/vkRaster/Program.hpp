#pragma once
#include <memory>

#include "vkRaster//ResourceManager.hpp"
#include "vkRaster/Renderer.hpp"
#include "vkRaster/types.hpp"
namespace vkr {
class System;
class Program {
   private:
    Size size{};
    const char* winName{};
    std::unique_ptr<System> system{std::make_unique<System>(size, winName)};

   protected:
    Renderer renderer{*system};
    virtual void onFrame(float deltaTime) = 0;

   public:
    Program(const Size& size, const char* name);
    ~Program();

    void run();
};
}  // namespace vkr