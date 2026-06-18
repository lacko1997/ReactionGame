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
    VkImageView colorView;
    VkFramebuffer frameBuffer;
    VkRenderPass renderPass;
}RenderImage;

typedef struct DepthBuffer
{
    VkImage depthImage;
    VkImageView depthView;
    VkDeviceMemory depthMemory;
}DepthBuffer;

typedef struct EngineBase
{
    DeviceBase devBase;
    VkSwapchainKHR swapchain;
    uint32_t queueFamilyIndex;
    VkQueue queue;
    uint32_t imageCount;
    DepthBuffer depth;
    RenderImage *renderImage;
    VkCommandPool cmdPool;
}EngineBase;

typedef struct Model
{
    uint32_t vertexCount;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
}Model;

typedef struct SceneMemory
{
    uint32_t textureCount;
    VkSampler *sampler;
    VkImage *textures;
    VkDeviceMemory *textureMemories;

    uint32_t uniformBufferCount;
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBufferMemories;

    uint32_t modelCount;
    Model *modelBuffers;
}SceneMemory;

typedef struct PipelineElements
{
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool  descrPool;
    VkDescriptorSet descriptors[2];
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
void makeDescriptorPool(EngineBase *base, RenderScene *scene);
void setCurrentScene(RenderScene *scene);
void *mainLoop(void* args);

void releaseSurfaceImages(EngineBase *base);


#endif //REACTIONGAME_VULKAN_RENDERER_H
