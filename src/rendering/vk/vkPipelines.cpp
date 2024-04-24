#include "vkPipelines.hpp"

#include <fstream>
#include <iostream>
#include <vector>

#include "vkUtils.hpp"

VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(
    VkShaderStageFlagBits stage, VkShaderModule shaderModule,
    const char* entry = "main") {
    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    // shader stage
    info.stage = stage;
    // module containing the code for this shader stage
    info.module = shaderModule;
    // the entry point of the shader
    info.pName = entry;
    return info;
}

VkShaderModule vk::loadShaderModule(const char* filePath, VkDevice device) {
    // open the file. With cursor at the end
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Cannot find file\n";
        return 0;
    }

    // find what the size of the file is by looking up the location of the
    // cursor because the cursor is at the end, it gives the size directly in
    // bytes
    size_t fileSize = (size_t)file.tellg();

    // spirv expects the buffer to be on uint32, so make sure to reserve a int
    // vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    // put file cursor at beginning
    file.seekg(0);

    // load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    // now that the file is loaded into the buffer, we can close it
    file.close();

    // create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    // codeSize has to be in bytes, so multply the ints in the buffer by size of
    // int to know the real size of the buffer
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    // check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
        VK_SUCCESS) {
        return 0;
    }
    return shaderModule;
}

VkPipelineLayoutCreateInfo vk::pipelineLayoutCreateInfo() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    // empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkRenderingAttachmentInfo vk::attachmentInfo(VkImageView view,
                                             VkClearValue* clear,
                                             VkImageLayout layout) {
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = view;
    colorAttachment.imageLayout = layout;
    colorAttachment.loadOp =
        clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if (clear) {
        colorAttachment.clearValue = *clear;
    }

    return colorAttachment;
}

VkRenderingAttachmentInfo vk::depthAttachmentInfo(VkImageView view,
                                                  VkImageLayout layout) {
    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.pNext = nullptr;

    depthAttachment.imageView = view;
    depthAttachment.imageLayout = layout;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil.depth = 1.f;

    return depthAttachment;
}

VkRenderingInfo vk::renderingInfo(VkExtent2D renderExtent,
                                  VkRenderingAttachmentInfo* colorAttachment,
                                  VkRenderingAttachmentInfo* depthAttachment) {
    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;

    renderInfo.renderArea = VkRect2D{VkOffset2D{0, 0}, renderExtent};
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = colorAttachment;
    renderInfo.pDepthAttachment = depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    return renderInfo;
}

void vk::PipelineBuilder::clear() {
    _inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

    _rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    _colorBlendAttachment = {};

    _multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    _pipelineLayout = {};

    _depthStencil = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

    _renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};

    _shaderStages.clear();
}

VkPipeline vk::PipelineBuilder::buildPipeline(VkDevice device) {
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineVertexInputStateCreateInfo _vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    _vertexInputInfo.pVertexAttributeDescriptions =
        vertexInputAttributes.data();
    _vertexInputInfo.vertexAttributeDescriptionCount =
        vertexInputAttributes.size();
    _vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
    _vertexInputInfo.vertexBindingDescriptionCount = vertexInputBindings.size();

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &_colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineInfo.pNext = &_renderInfo;

    pipelineInfo.stageCount = (uint32_t)_shaderStages.size();
    pipelineInfo.pStages = _shaderStages.data();
    pipelineInfo.pVertexInputState = &_vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &_inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &_rasterizer;
    pipelineInfo.pMultisampleState = &_multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &_depthStencil;
    pipelineInfo.layout = _pipelineLayout;

    VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT,
                              VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineInfo.pDynamicState = &dynamicInfo;

    VkPipeline newPipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &newPipeline) != VK_SUCCESS) {
        std::cerr << "failed to create pipeline";
        exit(-1);
    }
    return newPipeline;
}

void vk::PipelineBuilder::setShaders(VkShaderModule vertexShader,
                                     VkShaderModule fragmentShader) {
    _shaderStages.clear();

    _shaderStages.push_back(pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

    _shaderStages.push_back(pipelineShaderStageCreateInfo(
        VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));
}

void vk::PipelineBuilder::setInputTopology(VkPrimitiveTopology topology) {
    _inputAssembly.topology = topology;
    _inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void vk::PipelineBuilder::setPolygonMode(VkPolygonMode mode) {
    _rasterizer.polygonMode = mode;
    _rasterizer.lineWidth = 1.f;
}

void vk::PipelineBuilder::setCullMode(VkCullModeFlags cullMode,
                                      VkFrontFace frontFace) {
    _rasterizer.cullMode = cullMode;
    _rasterizer.frontFace = frontFace;
}

void vk::PipelineBuilder::setMultisamplingNone() {
    _multisampling.sampleShadingEnable = VK_FALSE;

    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f;
    _multisampling.pSampleMask = nullptr;

    _multisampling.alphaToCoverageEnable = VK_FALSE;
    _multisampling.alphaToOneEnable = VK_FALSE;
}

void vk::PipelineBuilder::disableBlending() {
    _colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_FALSE;
}

void vk::PipelineBuilder::enableBlendingAdditive() {
    _colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_TRUE;
    _colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    _colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
    _colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    _colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    _colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    _colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void vk::PipelineBuilder::enableBlendingAlphablend() {
    _colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_TRUE;
    _colorBlendAttachment.srcColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    _colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    _colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    _colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    _colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    _colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
}

void vk::PipelineBuilder::setColorAttachmentFormat(VkFormat format) {
    _colorAttachmentformat = format;
    _renderInfo.colorAttachmentCount = 1;
    _renderInfo.pColorAttachmentFormats = &_colorAttachmentformat;
}

void vk::PipelineBuilder::setDepthFormat(VkFormat format) {
    _renderInfo.depthAttachmentFormat = format;
}

void vk::PipelineBuilder::disableDepthtest() {
    _depthStencil.depthTestEnable = VK_FALSE;
    _depthStencil.depthWriteEnable = VK_FALSE;
    _depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    _depthStencil.depthBoundsTestEnable = VK_FALSE;
    _depthStencil.stencilTestEnable = VK_FALSE;
    _depthStencil.front = {};
    _depthStencil.back = {};
    _depthStencil.minDepthBounds = 0.f;
    _depthStencil.maxDepthBounds = 1.f;
}

void vk::PipelineBuilder::enableDepthtest(bool depthWriteEnable,
                                          VkCompareOp op) {
    _depthStencil.depthTestEnable = VK_TRUE;
    _depthStencil.depthWriteEnable = depthWriteEnable;
    _depthStencil.depthCompareOp = op;
    _depthStencil.depthBoundsTestEnable = VK_FALSE;
    _depthStencil.stencilTestEnable = VK_FALSE;
    _depthStencil.front = {};
    _depthStencil.back = {};
    _depthStencil.minDepthBounds = 0.f;
    _depthStencil.maxDepthBounds = 1.f;
}