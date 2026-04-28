//
// Created by laszlo on 4/1/26.
//
#include <string.h>
#include "scene_elements.h"

static float pushConstant[16] =
        {
                1.0f,  0.0f, 0.0f, 0.0f,
                0.0f, -1.0f, 0.0f, 0.0f,
                0.0f,  0.0f, 0.5f, 0.0f,
                0.0f,  0.0f, 0.0f, 1.0f
        };

static VkResult createTextureImage(EngineBase *base, SceneMemory *memory, uint32_t width, uint32_t height, uint32_t index);
static VkResult createSampler(EngineBase *base, RenderScene *scene, uint32_t samplerIndex);
static VkResult createBuffer(EngineBase *base, RenderScene *scene);

void makeTexture(EngineBase *base, RenderScene *scene, uint32_t textureCount, TextureInfo *info)
{
    for(uint32_t i = 0; i < textureCount; i++)
    {
        void *data;
        createTextureImage(base, &scene->memory, info->width, info->height, i);
        vkMapMemory(base->devBase.device, scene->memory.textureMemories[i], 0, info->width * info->height * 4, 0, &data);
        memcpy(data, info->data, info->width * info->height * 4);
        vkUnmapMemory(base->devBase.device, scene->memory.textureMemories[i]);
        createSampler(base, scene, i);
    }
}

void makeVertexBuffer(EngineBase *base, RenderScene *scene, float *vertexData, uint32_t floatCount)
{
    create
};

static VkResult createTextureImage(EngineBase *base, SceneMemory *memory, uint32_t width, uint32_t height, uint32_t index)
{
    uint32_t  i;
    VkResult result;
    VkExtent3D imgSize;
    imgSize.width = width;
    imgSize.height = height;
    imgSize.depth = 1;

    VkImageCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.arrayLayers = 1;
    info.extent = imgSize;
    info.format = VK_FORMAT_R8G8B8A8_UINT;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &base->queueFamilyIndex;
    info.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.mipLevels = 1;
    info.tiling = VK_IMAGE_TILING_LINEAR;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    result = vkCreateImage(base->devBase.device, &info, NULL, &memory->textures[index]);

    if(result == VK_SUCCESS)
    {
        VkPhysicalDeviceMemoryProperties memProps;
        VkMemoryRequirements reqs;
        uint32_t memTypeIndex = 0;

        vkGetPhysicalDeviceMemoryProperties(base->devBase.gpu, &memProps);
        for(i = 0; i < memProps.memoryTypeCount; i++)
        {
            if(memProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            {
                memTypeIndex = i;
                break;
            }
        }

        vkGetImageMemoryRequirements(base->devBase.device, memory->textures[index], &reqs);

        VkMemoryAllocateInfo allocInfo;
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = NULL;
        allocInfo.allocationSize = reqs.size;
        allocInfo.memoryTypeIndex = memTypeIndex;

        result = vkAllocateMemory(base->devBase.device, &allocInfo, NULL, &memory->textureMemories[index]);
        if(result == VK_SUCCESS)
        {
            result = vkBindImageMemory(base->devBase.device, memory->textures[index], memory->textureMemories[index], 0);
        }
    }
    return result;
}

static VkResult createSampler(EngineBase *base, RenderScene *scene, uint32_t samplerIndex)
{
    VkSamplerCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    info.anisotropyEnable = VK_FALSE;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    info.compareEnable = VK_FALSE;
    info.compareOp = VK_COMPARE_OP_ALWAYS;
    info.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    info.maxAnisotropy = 0.0f;
    info.mipLodBias = 0.0f;
    info.maxLod = VK_LOD_CLAMP_NONE;
    info.minLod = 1.0f;
    info.unnormalizedCoordinates = VK_FALSE;

    return vkCreateSampler(base->devBase.device, &info, NULL, &scene->memory.sampler[samplerIndex]);
}

static VkResult createBuffer(EngineBase *base, RenderScene *scene, VkBufferUsageFlags usage, uint32_t floatCount, uint32_t index)
{
    VkResult result;
    VkBufferCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &base->queueFamilyIndex;
    info.size = sizeof(float)*floatCount;

    result = vkCreateBuffer(base->devBase.device, &info, NULL, &scene->memory.vertexBuffers[index]);

    if(result == VK_SUCCESS)
    {
        uint32_t typeIndex = 0;
        VkPhysicalDeviceMemoryProperties memProps;
        VkMemoryRequirements reqs;

        vkGetPhysicalDeviceMemoryProperties(base->devBase.gpu, &memProps);

        vkGetBufferMemoryRequirements(base->devBase.device, scene->memory.vertexBuffers[index], &reqs);

        for(uint32_t  i = 0; i < memProps.memoryTypeCount; i++)
        {
            if((memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
            {
                typeIndex = 0;
            }
        }

        VkMemoryAllocateInfo memInfo;
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.pNext = NULL;
        memInfo.memoryTypeIndex = typeIndex;
        memInfo.allocationSize = reqs.size;

        result = vkAllocateMemory(base->devBase.device, &memInfo, NULL, &scene->memory.vertexBufferMemories[index]);
        if(result == VK_SUCCESS)
        {
            result = vkBindBufferMemory(base->devBase.device, scene->memory.vertexBufferMemories[index], scene->memory.vertexBuffers[index], 0);
        }
    }

    return result;
}

static VkResult createDescriptorSetLayout(EngineBase *base, RenderScene *scene)
{
    VkDescriptorSetLayoutBinding binding[2];
    binding[0].descriptorCount = 1;
    binding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding[0].binding = 0;
    binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding[0].pImmutableSamplers = NULL;

    binding[1].descriptorCount = 1;
    binding[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
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

static VkResult createPipelineLayout(EngineBase *base, RenderScene *scene)
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
static VkResult createShaderModule(EngineBase *base, RenderScene *scene, uint32_t index, Code *codes)
{
    VkShaderModuleCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.codeSize = codes->codeSize;
    info.pCode = codes->code;

    return vkCreateShaderModule(base->devBase.device, &info, NULL, &scene->pipeline.modules[index]);
}

static VkResult createPipeline(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
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

    VkPipelineShaderStageCreateInfo stages[2];
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].flags = 0;
    stages[0].pNext = NULL;
    stages[0].module = scene->pipeline.modules[0];
    stages[0].pName = "vertex";
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].pSpecializationInfo = NULL;

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].flags = 0;
    stages[1].pNext = NULL;
    stages[1].module = scene->pipeline.modules[1];
    stages[1].pName = "fragment";
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].pSpecializationInfo = NULL;

    VkVertexInputAttributeDescription attribs[2];
    attribs[0].location = 0;
    attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribs[0].binding = 0;
    attribs[0].offset = 0;

    attribs[1].location = 1;
    attribs[1].binding = 0;
    attribs[1].offset = 3 * sizeof(float);
    attribs[1].format = VK_FORMAT_R32G32_SFLOAT;

    attribs[2].location = 1;
    attribs[2].binding = 0;
    attribs[2].offset = 0;
    attribs[2].format = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;

    attribs[3].location = 1;
    attribs[3].binding = 0;
    attribs[3].offset = 16 * sizeof(float);
    attribs[3].format = VK_FORMAT_R32G32_SFLOAT;

    VkVertexInputBindingDescription bindings[2];
    bindings[0].stride = 5 * sizeof(float);
    bindings[0].binding = 0;
    bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindings[1].stride = 5 * sizeof(float);
    bindings[1].binding = 0;
    bindings[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    VkPipelineVertexInputStateCreateInfo input;
    input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    input.flags = 0;
    input.pNext = NULL;
    input.vertexAttributeDescriptionCount = 4;
    input.pVertexAttributeDescriptions = attribs;
    input.vertexBindingDescriptionCount = 2;
    input.pVertexBindingDescriptions = bindings;

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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasSlopeFactor = 0.0;
    rasterizer.depthBiasConstantFactor = 0.01;
    rasterizer.lineWidth = 1.0f;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.depthClampEnable = VK_TRUE;
    rasterizer.depthBiasClamp = 0.1f;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkSampleMask sampleMask = 0;
    VkPipelineMultisampleStateCreateInfo multisample;
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.flags = 0;
    multisample.pNext = NULL;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;
    multisample.minSampleShading = 0.0f;
    multisample.pSampleMask = &sampleMask;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;

    VkStencilOpState back;
    back.compareMask = 0;
    back.compareOp = VK_COMPARE_OP_ALWAYS;
    back.depthFailOp = VK_STENCIL_OP_REPLACE;
    back.failOp = VK_STENCIL_OP_KEEP;
    back.passOp = VK_STENCIL_OP_KEEP;
    back.writeMask = 0;
    back.reference = 0;

    VkStencilOpState front;
    back.compareMask = 0;
    back.compareOp = VK_COMPARE_OP_ALWAYS;
    back.depthFailOp = VK_STENCIL_OP_REPLACE;
    back.failOp = VK_STENCIL_OP_KEEP;
    back.passOp = VK_STENCIL_OP_KEEP;
    back.writeMask = 0;
    back.reference = 0;

    VkPipelineDepthStencilStateCreateInfo depthInfo;
    depthInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthInfo.flags = 0;
    depthInfo.pNext = NULL;
    depthInfo.depthBoundsTestEnable = VK_FALSE;
    depthInfo.minDepthBounds = 0.1f;
    depthInfo.maxDepthBounds = 1000.0f;
    depthInfo.depthTestEnable = VK_TRUE;
    depthInfo.depthWriteEnable = VK_TRUE;
    depthInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthInfo.stencilTestEnable = VK_FALSE;
    depthInfo.back = back;
    depthInfo.front = front;

    VkPipelineColorBlendAttachmentState attachment;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    attachment.blendEnable = VK_TRUE;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.colorWriteMask = 0x0F;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

    VkPipelineColorBlendStateCreateInfo colorBlend;
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
    dynInfo.flags = 0;
    dynInfo.pNext = NULL;
    dynInfo.dynamicStateCount = 0;
    dynInfo.pDynamicStates = NULL;

    VkGraphicsPipelineCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.flags = NULL;
    info.layout = scene->pipeline.pipelineLayout;
    info.renderPass = base->renderImage->renderPass;
    info.basePipelineHandle = VK_NULL_HANDLE;
    info.basePipelineIndex = -1;
    info.subpass = 0;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &input;
    info.pInputAssemblyState = &inputAsm;
    info.pTessellationState = &tescInfo;
    info.pViewportState = &viewportInfo;
    info.pRasterizationState = &rasterizer;
    info.pMultisampleState = &multisample;
    info.pDepthStencilState = &depthInfo;
    info.pColorBlendState = &colorBlend;
    info.pDynamicState = &dynInfo;

    return vkCreateGraphicsPipelines(base->devBase.device, VK_NULL_HANDLE, 1, &info, NULL, &scene->pipeline.pipeline);
}

