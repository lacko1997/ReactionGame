//
// Created by laszlo on 4/1/26.
//
#include "scene_elements.h"

#include <string.h>
#include <android/log.h>

static VkResult createTextureImage(EngineBase *base, SceneMemory *memory, uint32_t width, uint32_t height, uint32_t index);
static VkResult createSampler(EngineBase *base, RenderScene *scene, uint32_t samplerIndex);
static VkResult createBuffer(EngineBase *base, RenderScene*, uint32_t size, uint32_t index, VkBufferUsageFlags usage, VkBool32 instanceBuffer);
static VkResult createVertexBuffer(EngineBase *base, RenderScene *scene, uint32_t vertexFloatCount,uint32_t instanceFloatCount, uint32_t index);
static VkResult createIndexBuffer(EngineBase *base, RenderScene *scene, uint32_t indexCount, uint32_t index);
static VkResult createUniformBuffer(EngineBase *base, RenderScene *scene, uint32_t floatCount, uint32_t index);
static VkResult createCommandBuffers(EngineBase *base, RenderScene *scene);
static VkResult createDescriptorSets(EngineBase *base, RenderScene *scene);

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

void makeVertexBuffer(EngineBase *base, RenderScene *scene, uint32_t index, float *vertexData, float *instanceData, uint32_t vertexFloatCount, uint32_t instanceFloatCount)
{
    void *data;
    CHECK_RESULT(createVertexBuffer(base, scene, vertexFloatCount, instanceFloatCount, index));
    vkMapMemory(base->devBase.device, scene->memory.modelBuffers[index].vertexBufferMemory, 0, sizeof(float) * vertexFloatCount, 0, &data);
    memcpy(data, vertexData, sizeof(float) * vertexFloatCount);
    vkUnmapMemory(base->devBase.device, scene->memory.modelBuffers[index].vertexBufferMemory);

    vkMapMemory(base->devBase.device, scene->memory.modelBuffers[index].instanceBufferMemory, 0, sizeof(float) * instanceFloatCount, 0, &data);
    memcpy(data, instanceData, sizeof(float) * instanceFloatCount);
    vkUnmapMemory(base->devBase.device, scene->memory.modelBuffers[index].instanceBufferMemory);
};

void makeUniformBuffer(EngineBase *base, RenderScene *scene, uint32_t floatCount, uint32_t index)
{
    CHECK_RESULT(createUniformBuffer(base, scene, floatCount, index));
}

void makeCommandBuffers(EngineBase *base, RenderScene *scene)
{
    CHECK_RESULT(createCommandBuffers(base, scene));
}

void makeDescriptorSets(EngineBase *base, RenderScene *scene)
{
    CHECK_RESULT(createDescriptorSets(base, scene));
}

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

static VkResult createBuffer(EngineBase *base, RenderScene *scene, uint32_t size, uint32_t index, VkBufferUsageFlags usage, VkBool32 instanceBuffer)
{
    VkResult result;
    VkBuffer *outputBuffer = NULL;
    VkDeviceMemory *outputMemory = NULL;

    if(usage == VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
    {
        outputBuffer = &scene->memory.modelBuffers[index].indexBuffer;
        outputMemory = &scene->memory.modelBuffers[index].indexBufferMemory;
    }
    else if(usage == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
    {
        if(instanceBuffer == VK_FALSE)
        {
            outputBuffer = &scene->memory.modelBuffers[index].vertexBuffer;
            outputMemory = &scene->memory.modelBuffers[index].vertexBufferMemory;
        }
        else
        {
            outputBuffer = &scene->memory.modelBuffers[index].instanceBuffer;
            outputMemory = &scene->memory.modelBuffers[index].instanceBufferMemory;
        }
    }
    else if(usage == VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
    {
        outputBuffer = &scene->memory.uniformBuffers[index];
        outputMemory = &scene->memory.uniformBufferMemories[index];
    }

    VkBufferCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.usage = usage;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &base->queueFamilyIndex;
    info.size = size;

    result = vkCreateBuffer(base->devBase.device, &info, NULL, outputBuffer);

    if(result == VK_SUCCESS)
    {
        uint32_t typeIndex = 0;
        VkPhysicalDeviceMemoryProperties memProps;
        VkMemoryRequirements reqs;

        vkGetPhysicalDeviceMemoryProperties(base->devBase.gpu, &memProps);

        vkGetBufferMemoryRequirements(base->devBase.device, *outputBuffer, &reqs);

        for(uint32_t  i = 0; i < memProps.memoryTypeCount; i++)
        {
            if((memProps.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != 0)
            {
                typeIndex = i;
                break;
            }
        }

        VkMemoryAllocateInfo memInfo;
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.pNext = NULL;
        memInfo.memoryTypeIndex = typeIndex;
        memInfo.allocationSize = reqs.size;

        result = vkAllocateMemory(base->devBase.device, &memInfo, NULL, outputMemory);
        if(result == VK_SUCCESS)
        {
            result = vkBindBufferMemory(base->devBase.device, *outputBuffer, *outputMemory, 0);
        }
    }

    return result;
}

static VkResult createVertexBuffer(EngineBase *base, RenderScene *scene, uint32_t vertexFloatCount, uint32_t instanceFloatCount, uint32_t index)
{
    VkResult result = createBuffer(base, scene, vertexFloatCount * sizeof(float), index, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_FALSE);
    if(result == VK_SUCCESS && instanceFloatCount > 0)
    {
        result = createBuffer(base, scene, instanceFloatCount * sizeof(float), index, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_TRUE);
    }
    return result;
}

static VkResult createUniformBuffer(EngineBase *base, RenderScene *scene, uint32_t floatCount, uint32_t index)
{
    return createBuffer(base, scene, floatCount * sizeof(float), index, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_FALSE);
}

static VkResult createIndexBuffer(EngineBase *base, RenderScene *scene, uint32_t indexCount, uint32_t index)
{
    return createBuffer(base, scene, index * sizeof(uint16_t), index, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_FALSE);
}

static VkResult createDescriptorSets(EngineBase *base, RenderScene *scene)
{
    VkResult result;

    VkDescriptorSetAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.pNext = NULL;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &scene->pipeline.descriptorSetLayout;
    info.descriptorPool = scene->pipeline.descrPool;

    result = vkAllocateDescriptorSets(base->devBase.device, &info, scene->pipeline.descriptors);
    if(result == VK_SUCCESS)
    {
        VkDescriptorBufferInfo writeBuffers[2];
        writeBuffers[0].offset = 0;
        writeBuffers[0].range = 64;
        writeBuffers[0].buffer = scene->memory.uniformBuffers[0];

        VkWriteDescriptorSet write[2];
        write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[0].pNext = NULL;
        write[0].descriptorCount = 1;
        write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write[0].pBufferInfo = &writeBuffers[0];
        write[0].dstArrayElement = 0;
        write[0].dstBinding = 0;
        write[0].dstSet = scene->pipeline.descriptors[0];
        write[0].pImageInfo = NULL;
        write[0].pTexelBufferView = NULL;

        writeBuffers[1].offset = 0;
        writeBuffers[1].range = 12;
        writeBuffers[1].buffer = scene->memory.uniformBuffers[1];

        write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write[1].pNext = NULL;
        write[1].descriptorCount = 1;
        write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write[1].pBufferInfo = &writeBuffers[1];
        write[1].dstArrayElement = 0;
        write[1].dstBinding = 1;
        write[1].dstSet = scene->pipeline.descriptors[1];
        write[1].pImageInfo = NULL;
        write[1].pTexelBufferView = NULL;

        vkUpdateDescriptorSets(base->devBase.device, 1, write, 0, NULL);
    }
    return result;
}

static VkResult createCommandBuffers(EngineBase *base, RenderScene *scene)
{
    VkCommandBufferAllocateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = NULL;
    info.commandPool = base->cmdPool;
    info.commandBufferCount = base->imageCount;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    scene->pipeline.cmdBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * base->imageCount);

    return vkAllocateCommandBuffers(base->devBase.device, &info, scene->pipeline.cmdBuffers);
}
