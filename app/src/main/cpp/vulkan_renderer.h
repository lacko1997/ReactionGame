//
// Created by lacko on 3/12/26.
//

#ifndef REACTIONGAME_VULKAN_RENDERER_H
#define REACTIONGAME_VULKAN_RENDERER_H

#define VK_USE_PLATFORM_ANDROID_KHR
#include <stdlib.h>
#include <vulkan/vulkan.h>

typedef struct DeviceBase
{
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSurfaceKHR surface;
}DeviceBase;

typedef struct RenderBase
{
    VkImage colorImage;
    VkImage depthImage;
    VkDeviceMemory depthMemory;
    VkImageView colorView;
    VkImageView depthView;
    VkFramebuffer frameBuffer;
    VkRenderPass renderPass;
}RenderImage;

typedef struct RenderRes
{
    VkCommandPool cmdPool;
    VkDescriptorPool descrPool;
}RenderRes;

typedef struct EngineBase
{
    DeviceBase devBase;
    VkSwapchainKHR swapchain;
    uint32_t queueFamilyIndex;
    VkQueue queue;
    uint32_t imageCount;
    RenderImage *renderImage;
    RenderRes renderRes;
}EngineBase;

typedef struct SceneMemory
{
    uint32_t textureCount;
    VkSampler *sampler;
    VkImage *textures;
    VkDeviceMemory *textureMemories;

    uint32_t uniformBufferCount;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBufferMemories;

    uint32_t vertexBufferCount;
    VkBuffer *vertexBuffers;
    VkDeviceMemory *vertexBufferMemories;
    uint32_t indexBufferCount;
    VkBuffer *indexBuffers;
    VkDeviceMemory *indexBufferMemories;
}SceneMemory;

typedef struct PipelineElements
{
    VkPipelineLayout pipelineLayout;
    VkDescriptorSet descriptors;
    VkDescriptorSetLayout descriptorSetLayout;
    VkShaderModule modules[2];
    VkPipeline *pipeline;
    VkCommandBuffer *cmdBuffers;
}PipelineElements;

typedef struct RenderScene
{
    PipelineElements pipeline;
    SceneMemory memory;
}RenderScene;

typedef struct TextureInfo
{
    uint32_t width;
    uint32_t height;
    uint8_t *data;
}TextureInfo;

#define CHECK_RESULT(RESULT)        \
    do {                            \
        if((RESULT) != VK_SUCCESS)  \
        {                           \
           __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "%s:%d, ERROR:%d fn:%s",__FILE__, __LINE__, RESULT, #RESULT); \
           return;                  \
        }                           \
    } while(0)

extern bool running;

void makeEngineBase(EngineBase *base);
void makeSurface(EngineBase *base, struct ANativeWindow *wnd);
void makeRenderImage(EngineBase *base, uint32_t width, uint32_t height);
void setCurrentScene(RenderScene *scene);
void *mainLoop(void* args);

void releaseSurfaceImages(EngineBase *base);


#endif //REACTIONGAME_VULKAN_RENDERER_H
