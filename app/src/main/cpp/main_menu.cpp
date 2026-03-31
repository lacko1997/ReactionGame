//
// Created by lacko on 3/28/26.
//
#include "main_menu.h"

#include "vulkan_renderer.h"

static float pushConstant[16] =
        {
            1.0f,  0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f,  0.0f, 0.5f, 0.0f,
            0.0f,  0.0f, 0.0f, 1.0f
        };

VkResult makeCommandBuffers(EngineBase *base, RenderRes *resource, RenderScene *scene)
{
    VkCommandBufferAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = resource->cmdPool;
    info.commandBufferCount = base->imageCount;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.pNext = NULL;

    scene->cmdBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * base->imageCount);

    return vkAllocateCommandBuffers(base->device, &info, scene->cmdBuffers);
}

VkResult createSampler(EngineBase *base, RenderScene *scene)
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

    return vkCreateSampler(base->device, &info, NULL, &scene->sampler);
}

VkResult createDescriptorSetLayout(EngineBase *base, RenderScene *scene)
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

    return vkCreateDescriptorSetLayout(base->device, &info, NULL, &scene->descriptorSetLayout);
}

VkResult createPipelineLayout()
{
    VkPushConstantRange range;
    range.size = 64;
    range.offset = 0;
    range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo info;
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &range;
    info.setLayoutCount = 1;
    info.pSetLayouts = NULL;

}

VkResult createPipeline()
{
    VkGraphicsPipelineCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.flags = NULL;
    //info.layout = layout;
    return !VK_SUCCESS;
}
