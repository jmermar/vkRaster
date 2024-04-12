#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "CommandsSubmitter.hpp"
#include "FrameData.hpp"
#include "MeshHandler.hpp"
#include "Swapchain.hpp"
#include "types.hpp"
#include "vk/vkBuffer.hpp"
#include "vk/vkDeletion.hpp"
#include "vk/vkDescriptors.hpp"
#include "vk/vkImage.hpp"
namespace vkr {
struct GPUDrawPushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBuffer;
};

class Renderer {
   private:
    class System {
       private:
        vk::vkSystem system;

       public:
        System(SDL_Window* win);
        ~System();
        System& operator=(const System&) = delete;
        System(const System&) = delete;

        inline const vk::vkSystem& get() { return system; }
    };

    Size screenSize{};
    SDL_Window* win{};

    System system{win};

    Swapchain swapchain{system.get(), screenSize.w, screenSize.h};
    FrameData frameData{system.get(), screenSize.w, screenSize.h};
    CommandsSubmitter submitter{system.get()};

   public:
    MeshHandler meshHandler{*this};

   private:
    vk::DescriptorAllocator globalDescriptorAllocator;

    VkPipelineLayout trianglePipelineLayout;
    VkPipeline trianglePipeline;

    vk::AllocatedImage drawImage;
    vk::AllocatedImage depthImage;

    VkFence submitFence{};
    VkCommandBuffer submitCommandBuffer{};

    uint32_t frameCounter{};

    void drawBackground(VkCommandBuffer cmd);
    void drawTriangle(VkCommandBuffer cmd);

    void initDescriptorSets();
    void initRenderPipeline();
    void initSubmit();

   public:
    Renderer(SDL_Window* window, uint32_t w, uint32_t h);
    ~Renderer();

    void render();

    GPUMesh uploadMesh(const MeshData& data);
    void destroyMesh(GPUMesh& mesh);
};
}  // namespace vkr