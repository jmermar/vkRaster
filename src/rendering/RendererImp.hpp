#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "CommandsSubmitter.hpp"
#include "FrameData.hpp"
#include "Swapchain.hpp"
#include "vk/vkBuffer.hpp"
#include "vk/vkDeletion.hpp"
#include "vk/vkDescriptors.hpp"
#include "vk/vkImage.hpp"
#include "vkRaster/types.hpp"
namespace vkr {
struct GPUMesh {
    vk::AllocatedBuffer indexBuffer;
    vk::AllocatedBuffer vertexBuffer;
    VkDeviceAddress vertexBufferAddr;
    size_t nVertices, nIndices;
};

struct GPUDrawPushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBuffer;
};

class RendererImp {
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

    vk::DescriptorAllocator globalDescriptorAllocator;

    VkPipelineLayout trianglePipelineLayout;
    VkPipeline trianglePipeline;

    vk::AllocatedImage drawImage;
    vk::AllocatedImage depthImage;

    VkFence submitFence{};
    VkCommandBuffer submitCommandBuffer{};

    uint32_t frameCounter{};

    std::unordered_map<std::string, GPUMesh> meshes;

    void drawBackground(VkCommandBuffer cmd);
    void drawTriangle(VkCommandBuffer cmd);

    void initDescriptorSets();
    void initRenderPipeline();
    void initSubmit();

   public:
    RendererImp(SDL_Window* window, uint32_t w, uint32_t h);
    ~RendererImp();

    void render();

    GPUMesh* uploadMesh(const std::string& name, const std::vector<uint32_t>& indices,
                       const std::vector<Vertex>& vertices);
    void destroyMesh(const std::string& name);
};
}  // namespace vkr