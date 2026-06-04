//
// Created by laszlo on 5/11/26.
//

#include "pipelines.h"

#include <string.h>
#include <android/log.h>
#include "vulkan_renderer.h"

#define MAX_NUM_OF_SPV_CODES 2

typedef struct SpirVCodes
{
    uint32_t codeSize;
    char *code;
}SpirVCodes;

static SpirVCodes codes[MAX_NUM_OF_SPV_CODES * 2];

static VkVertexInputAttributeDescription attribs[5];
static VkVertexInputBindingDescription bindings[2];
static VkPipelineVertexInputStateCreateInfo inputAttribs;
static VkPipelineShaderStageCreateInfo shaderStages[2];
static VkPipelineDepthStencilStateCreateInfo depthInfo;

static VkResult createShaderModule(EngineBase *base, RenderScene *scene, uint32_t index);
static VkResult createModelDescriptorSetLayout(EngineBase *base, RenderScene *scene);
static VkResult createPipeline(EngineBase *base, RenderScene *scene, uint32_t index, uint32_t width, uint32_t height);

static VkResult createTextPipelineLayout(EngineBase *base, RenderScene *scene);
static VkResult createModelPipelineLayout(EngineBase *base, RenderScene *scene);

static void fillTextPipelineInputAttribs();
static void fillModelPipelineInputAttribs();

static void fillTextPipelineShaderStages(RenderScene *scene);
static void fillModelPipelineShaderStages(RenderScene *scene);

static void fillTextPipelineDepthStencilStage();
static void fillModelPipelineDepthStencilStage();

void initCodesArray()
{
    memset(codes, 0, sizeof(codes));
}

void putSpvCode(uint32_t index, char* byteArray, uint32_t byteCount)
{
    codes[index].codeSize = byteCount;

    codes[index].code = (char*)malloc(byteCount);
    memcpy((char*)codes[index].code, (char*)byteArray, byteCount);
}

void makeModelPipeline(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    createModelDescriptorSetLayout(base, scene);
    createModelPipelineLayout(base, scene);
    fillModelPipelineInputAttribs();
    CHECK_RESULT(createShaderModule(base, scene, MODEL_SHADER_MODULE_INDEX * 2 + 0));
    CHECK_RESULT(createShaderModule(base, scene, MODEL_SHADER_MODULE_INDEX * 2 + 1));
    fillModelPipelineShaderStages(scene);
    fillModelPipelineDepthStencilStage();
    scene->pipeline.pipeline = (VkPipeline*)malloc(sizeof(VkPipeline) *  base->imageCount);
    for(uint32_t i = 0; i < base->imageCount; i++)
    {
        CHECK_RESULT(createPipeline(base, scene, i, width, height));
    }
}

void releasePipeline(EngineBase *base, RenderScene *scene)
{
    for(uint32_t i = 0; i < base->imageCount; i++)
    {
        vkDestroyPipeline(base->devBase.device, scene->pipeline.pipeline[i], NULL);
    }
}

static VkResult createModelDescriptorSetLayout(EngineBase *base, RenderScene *scene)
{
    VkDescriptorSetLayoutBinding binding[2];
    binding[0].descriptorCount = 1;
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding[0].binding = 0;
    binding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    binding[0].pImmutableSamplers = NULL;

    binding[1].descriptorCount = 1;
    binding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding[1].binding = 1;
    binding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding[1].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.bindingCount = 2;
    info.pBindings = binding;

    return vkCreateDescriptorSetLayout(base->devBase.device, &info, NULL, &scene->pipeline.descriptorSetLayout);
}

static void fillTextPipelineInputAttribs()
{
    attribs[0].location = 0;
    attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribs[0].binding = 0;
    attribs[0].offset = 0;

    attribs[1].location = 1;
    attribs[1].binding = 0;
    attribs[1].offset = 2 * sizeof(float);
    attribs[1].format = VK_FORMAT_R32G32_SFLOAT;

    attribs[2].location = 1;
    attribs[2].binding = 1;
    attribs[2].offset = 3 * sizeof(float);
    attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribs[3].location = 1;
    attribs[3].binding = 1;
    attribs[3].offset = 3 * sizeof(float);
    attribs[3].format = VK_FORMAT_R32G32B32_SFLOAT;

    attribs[4].location = 1;
    attribs[4].binding = 1;
    attribs[4].offset = 3 * sizeof(float);
    attribs[4].format = VK_FORMAT_R32G32B32_SFLOAT;

    bindings[0].binding = 0;
    bindings[0].stride = 4 * sizeof(float);
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindings[1].binding = 1;
    bindings[1].stride = 9 * sizeof(float);
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    inputAttribs.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputAttribs.flags = 0;
    inputAttribs.pNext = NULL;
    inputAttribs.vertexAttributeDescriptionCount = 5;
    inputAttribs.pVertexAttributeDescriptions = attribs;
    inputAttribs.vertexBindingDescriptionCount = 2;
    inputAttribs.pVertexBindingDescriptions = bindings;
}

static void fillModelPipelineInputAttribs()
{
    attribs[0].location = 0;
    attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribs[0].binding = 0;
    attribs[0].offset = 0;

    attribs[1].location = 1;
    attribs[1].binding = 0;
    attribs[1].offset = 3 * sizeof(float);
    attribs[1].format = VK_FORMAT_R32G32_SFLOAT;

    attribs[2].location = 2;
    attribs[2].binding = 0;
    attribs[2].offset = 5 * sizeof(float);
    attribs[2].format = VK_FORMAT_R32G32B32_SFLOAT;

    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindings[0].stride = 8 * sizeof(float);
    bindings[0].binding = 0;

    inputAttribs.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    inputAttribs.pNext = NULL;
    inputAttribs.flags = 0;
    inputAttribs.vertexBindingDescriptionCount = 1;
    inputAttribs.pVertexBindingDescriptions = bindings;
    inputAttribs.vertexAttributeDescriptionCount = 3;
    inputAttribs.pVertexAttributeDescriptions = attribs;
}

static void fillModelPipelineShaderStages(RenderScene *scene)
{
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = NULL;
    shaderStages[0].module = scene->pipeline.modules[0];
    shaderStages[0].pName = "main";
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pSpecializationInfo = NULL;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = NULL;
    shaderStages[1].module = scene->pipeline.modules[1];
    shaderStages[1].pName = "main";
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pSpecializationInfo = NULL;
}

static void fillTextPipelineShaderStages(RenderScene *scene)
{
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = NULL;
    shaderStages[0].module = scene->pipeline.modules[0];
    shaderStages[0].pName = "main";
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].pSpecializationInfo = NULL;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = NULL;
    shaderStages[1].module = scene->pipeline.modules[1];
    shaderStages[1].pName = "main";
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].pSpecializationInfo = NULL;
}

static void fillModelPipelineDepthStencilStage()
{
    VkStencilOpState back;
    back.compareMask = 0;
    back.compareOp = VK_COMPARE_OP_LESS;
    back.depthFailOp = VK_STENCIL_OP_KEEP;
    back.failOp = VK_STENCIL_OP_KEEP;
    back.passOp = VK_STENCIL_OP_KEEP;
    back.writeMask = 0;
    back.reference = 0;

    VkStencilOpState front;
    front.compareMask = 0;
    front.compareOp = VK_COMPARE_OP_LESS;
    front.depthFailOp = VK_STENCIL_OP_KEEP;
    front.failOp = VK_STENCIL_OP_KEEP;
    front.passOp = VK_STENCIL_OP_KEEP;
    front.writeMask = 0;
    front.reference = 0;

    depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthInfo.flags = 0;
    depthInfo.pNext = NULL;
    depthInfo.depthBoundsTestEnable = VK_FALSE;
    depthInfo.depthTestEnable = VK_TRUE;
    depthInfo.depthWriteEnable = VK_TRUE;
    depthInfo.minDepthBounds = 0.0f;
    depthInfo.maxDepthBounds = 1.0f;
    depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthInfo.stencilTestEnable = VK_FALSE;
    depthInfo.back = back;
    depthInfo.front = front;
}

static VkResult createModelPipelineLayout(EngineBase *base, RenderScene *scene)
{
    VkPushConstantRange range;
    range.size = 64;
    range.offset = 0;
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &range;
    info.setLayoutCount = 1;
    info.pSetLayouts = &scene->pipeline.descriptorSetLayout;

    return vkCreatePipelineLayout(base->devBase.device, &info, NULL, &scene->pipeline.pipelineLayout);
}

static VkResult createShaderModule(EngineBase *base, RenderScene *scene, uint32_t index)
{
    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.codeSize = codes[index].codeSize;
    info.pCode = (uint32_t*)codes[index].code;

    return vkCreateShaderModule(base->devBase.device, &info, NULL, &scene->pipeline.modules[index % 2]);
}

static VkResult createPipeline(EngineBase *base, RenderScene *scene, uint32_t index, uint32_t width, uint32_t height)
{
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = width;
    scissor.extent.height = height;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)width;
    viewport.height = (float)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkPipelineInputAssemblyStateCreateInfo inputAsm;
    inputAsm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAsm.flags = 0;
    inputAsm.pNext = NULL;
    inputAsm.primitiveRestartEnable = VK_FALSE;
    inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineTessellationStateCreateInfo tescInfo;
    tescInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    tescInfo.flags = 0;
    tescInfo.pNext = NULL;
    tescInfo.patchControlPoints = 0;

    VkPipelineViewportStateCreateInfo viewportInfo;
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.flags = 0;
    viewportInfo.pNext = NULL;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;

    VkPipelineRasterizationStateCreateInfo rasterizer;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.flags = 0;
    rasterizer.pNext = nullptr;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasSlopeFactor = 0.0;
    rasterizer.depthBiasConstantFactor = 0.01;
    rasterizer.lineWidth = 1.0f;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.depthBiasClamp = 0.01f;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkSampleMask sampleMask = 0xFF;
    VkPipelineMultisampleStateCreateInfo multisample;
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = NULL;
    multisample.flags = 0;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 0;
    multisample.pSampleMask = &sampleMask;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState attachment;
    attachment.blendEnable = VK_FALSE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    attachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo colorBlend;
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.pNext = NULL;
    colorBlend.flags = 0;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = VK_LOGIC_OP_EQUIVALENT;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &attachment;
    colorBlend.blendConstants[0] = 0.0f;
    colorBlend.blendConstants[1] = 0.0f;
    colorBlend.blendConstants[2] = 0.0f;
    colorBlend.blendConstants[3] = 0.0f;

    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.flags = 0;
    colorBlend.pNext = NULL;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &attachment;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = VK_LOGIC_OP_EQUIVALENT;
    colorBlend.blendConstants[0] = 0.0f;
    colorBlend.blendConstants[1] = 0.0f;
    colorBlend.blendConstants[2] = 0.0f;
    colorBlend.blendConstants[3] = 0.0f;

    VkPipelineDynamicStateCreateInfo dynInfo;
    dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynInfo.pNext = NULL;
    dynInfo.flags = 0;
    dynInfo.dynamicStateCount = 0;
    dynInfo.pDynamicStates = NULL;

    VkGraphicsPipelineCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.stageCount = 2;
    info.pStages = shaderStages;
    info.pVertexInputState = &inputAttribs;
    info.pInputAssemblyState = &inputAsm;
    info.pTessellationState = &tescInfo;
    info.pViewportState = &viewportInfo;
    info.pRasterizationState = &rasterizer;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depthInfo;
    info.pColorBlendState = &colorBlend;
    info.pDynamicState = &dynInfo;
    info.layout = scene->pipeline.pipelineLayout;
    info.renderPass = base->renderImage[index].renderPass;
    info.subpass = 0;
    info.basePipelineHandle = VK_NULL_HANDLE;
    info.basePipelineIndex = -1;

    return vkCreateGraphicsPipelines(base->devBase.device, VK_NULL_HANDLE, 1, &info, NULL, &scene->pipeline.pipeline[index]);
}
