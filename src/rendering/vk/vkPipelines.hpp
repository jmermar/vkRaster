#pragma once

#include <vector>

#include "vulkan/vulkan.h"
namespace vk {
VkShaderModule loadShaderModule(const char* file, VkDevice device);

class PipelineBuilder {
    //> pipeline
   public:
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    VkPipelineRasterizationStateCreateInfo _rasterizer;
    VkPipelineColorBlendAttachmentState _colorBlendAttachment;
    VkPipelineMultisampleStateCreateInfo _multisampling;
    VkPipelineLayout _pipelineLayout;
    VkPipelineDepthStencilStateCreateInfo _depthStencil;
    VkPipelineRenderingCreateInfo _renderInfo;
    VkFormat _colorAttachmentformat;

    std::vector<VkVertexInputBindingDescription> vertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

    PipelineBuilder() { clear(); }

    void clear();

    VkPipeline buildPipeline(VkDevice device);
    //< pipeline
    void setShaders(VkShaderModule vertexShader, VkShaderModule fragmentShader);
    void setInputTopology(VkPrimitiveTopology topology);
    void setPolygonMode(VkPolygonMode mode);
    void setCullMode(VkCullModeFlags cullMode, VkFrontFace frontFace);
    void setMultisamplingNone();
    void disableBlending();
    void enableBlendingAdditive();
    void enableBlendingAlphablend();

    void setColorAttachmentFormat(VkFormat format);
    void setDepthFormat(VkFormat format);
    void disableDepthtest();
    void enableDepthtest(bool depthWriteEnable, VkCompareOp op);
};

VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo();

VkRenderingAttachmentInfo attachmentInfo(VkImageView view, VkClearValue* clear,
                                         VkImageLayout layout);
VkRenderingAttachmentInfo depthAttachmentInfo(VkImageView view,
                                              VkImageLayout layout);
VkRenderingInfo renderingInfo(VkExtent2D renderExtent,
                              VkRenderingAttachmentInfo* colorAttachment,
                              VkRenderingAttachmentInfo* depthAttachment);

}  // namespace vk