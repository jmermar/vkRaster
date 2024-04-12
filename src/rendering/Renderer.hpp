#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "CommandsSubmitter.hpp"
#include "FrameData.hpp"
#include "MeshHandler.hpp"
#include "Swapchain.hpp"
#include "renderers/UnlitRenderer.hpp"
#include "types.hpp"
#include "vk/vkBuffer.hpp"
#include "vk/vkDeletion.hpp"
#include "vk/vkDescriptors.hpp"
#include "vk/vkImage.hpp"
namespace vkr {
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
    UnlitRenderer unlitRenderer{*this};

   private:
    vk::DescriptorAllocator globalDescriptorAllocator;

    vk::AllocatedImage drawImage;
    vk::AllocatedImage depthImage;

    VkFence submitFence{};
    VkCommandBuffer submitCommandBuffer{};

    uint32_t frameCounter{};

    void drawBackground(VkCommandBuffer cmd);

    void initDescriptorSets();
    void initSubmit();

   public:
    Renderer(SDL_Window* window, uint32_t w, uint32_t h);
    ~Renderer();

    struct {
        glm::mat4 project;
        glm::mat4 view;
    } renderData;

    void render();

    const Size& getScreenSize() { return screenSize; }

    const vk::vkSystem& getSystem() { return system.get(); }
    const vk::AllocatedImage& getImage() { return drawImage; }
    const vk::AllocatedImage& getDepthImage() { return depthImage; }

    vk::AllocatedBuffer uploadBuffer(void* data, size_t size,
                                     VkBufferUsageFlags usage);
    GPUMesh uploadMesh(const MeshData& data);
    void destroyMesh(GPUMesh& mesh);
    void destroyBuffer(vk::AllocatedBuffer& buffer);
};
}  // namespace vkr