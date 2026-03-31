//
// Created by lacko on 3/12/26.
//

#ifndef REACTIONGAME_VULKAN_RENDERER_H
#define REACTIONGAME_VULKAN_RENDERER_H

#define VK_USE_PLATFORM_ANDROID_KHR
#include <stdlib.h>
#include <vulkan/vulkan.h>

typedef struct RenderBase
{
    VkImage colorImage;
    VkImage depthImage;
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
    VkInstance instance;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    uint32_t imageCount;
    RenderImage *renderImage;
    RenderRes renderRes;
}EngineBase;

typedef struct RenderScene
{
    VkPipeline pipeline;
    VkDescriptorSet descriptors;
    VkCommandBuffer *cmdBuffers;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    uint32_t samplerCount;
    VkSampler *sampler;
}RenderScene;


void makeEngineBase(EngineBase *base);
void makeSurface(EngineBase *base, struct ANativeWindow *wnd);
void makeRenderImage(EngineBase *base, uint32_t width, uint32_t height);

#endif //REACTIONGAME_VULKAN_RENDERER_H
