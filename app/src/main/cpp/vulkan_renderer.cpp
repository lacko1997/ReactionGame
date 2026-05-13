//
// Created by lacko on 3/12/26.
//
#include "vulkan_renderer.h"

#include <android/log.h>
#include <android/window.h>

RenderScene *currentScene;

static void createDebugMessenger(EngineBase *base);

static VkResult createInstance(EngineBase *base);
static VkResult createDevice(EngineBase *base);
static VkResult createSurface(EngineBase *base, struct ANativeWindow *wnd);
static VkResult createSwapchain(EngineBase *base, uint32_t width, uint32_t height);

static VkResult createImageViews(EngineBase *base, uint32_t index);
static VkResult createDepthImage(EngineBase *base, uint32_t width, uint32_t height, uint32_t index);
static VkResult createFramebuffer(EngineBase *base, uint32_t width, uint32_t height, uint32_t index);
static VkResult createRenderpass(EngineBase *base, uint32_t index);
static VkResult createCommandPool(EngineBase *base);
static VkResult createDescriptorPool(EngineBase *base);

VkDebugUtilsMessengerEXT messenger;
#define DEBUG
void makeEngineBase(EngineBase *base)
{
    __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "Assert OK.");
    CHECK_RESULT(createInstance(base));
#ifdef DEBUG
    createDebugMessenger(base);
#endif
    CHECK_RESULT(createDevice(base));
    CHECK_RESULT(createCommandPool(base));
}

void makeSurface(EngineBase *base, struct ANativeWindow *wnd)
{
    createSurface(base, wnd);
}

void makeRenderImage(EngineBase *base, uint32_t width, uint32_t height)
{
    uint32_t i;
    __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "Assert OK.");
    CHECK_RESULT(createSwapchain(base, width, height));
    for(i = 0; i < base->imageCount; i++)
    {
        CHECK_RESULT(createDepthImage(base, width, height, i));
        CHECK_RESULT(createImageViews(base, i));
        CHECK_RESULT(createRenderpass(base, i));
        CHECK_RESULT(createFramebuffer(base, width, height, i));
    }
}

static VkResult createDepthImage(EngineBase *base, uint32_t width, uint32_t height, uint32_t index)
{
    uint32_t i;
    uint32_t heapIndex;
    VkDeviceMemory memory;
    VkResult result;

    VkExtent3D extent;
    extent.width = width;
    extent.height = height;
    extent.depth = 1;

    VkImageCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.flags = 0;
    info.pNext = NULL;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.extent = extent;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &base->queueFamilyIndex;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.mipLevels = 1;
    info.format = VK_FORMAT_D32_SFLOAT;
    info.arrayLayers = 1;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    result = vkCreateImage(base->devBase.device, &info, NULL, &base->renderImage[index].depthImage);

    if(result == VK_SUCCESS)
    {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(base->devBase.device, base->renderImage[index].depthImage, &memReqs);

        VkPhysicalDeviceMemoryProperties props;
        vkGetPhysicalDeviceMemoryProperties(base->devBase.gpu, &props);

        for (i = 0; i < props.memoryTypeCount; i++)
        {
            heapIndex = props.memoryTypes[i].heapIndex;
            if ((props.memoryHeaps[heapIndex].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
            {
                break;
            }
        }

        VkMemoryAllocateInfo memInfo;
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = memReqs.size;
        memInfo.pNext = NULL;
        memInfo.memoryTypeIndex = i;

        result = vkAllocateMemory(base->devBase.device, &memInfo, NULL, &memory);

        if(result == VK_SUCCESS)
        {
            result = vkBindImageMemory(base->devBase.device, base->renderImage[index].depthImage, memory, 0);
        }
    }
    return result;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *userData)
{
    switch(severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            __android_log_print(ANDROID_LOG_FATAL,"Vulkan INFO", "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            __android_log_print(ANDROID_LOG_FATAL,"Vulkan VERBOSE", "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            __android_log_print(ANDROID_LOG_FATAL,"Vulkan WARN", "%s", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            __android_log_print(ANDROID_LOG_FATAL,"Vulkan ERROR", "%s", pCallbackData->pMessage);
            break;
        default:
            break;
    }
    return VK_FALSE;
}

static void createDebugMessenger(EngineBase *base)
{
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(base->devBase.instance, "vkCreateDebugUtilsMessengerEXT");

    VkDebugUtilsMessengerCreateInfoEXT info;
    info.flags = 0;
    info.pNext = NULL;
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    info.pfnUserCallback = debugCallback;

    vkCreateDebugUtilsMessengerEXT(base->devBase.instance, &info, NULL, &messenger);
}


static VkResult createInstance(EngineBase *base)
{
#ifdef DEBUG
    const char *extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME, "VK_EXT_debug_report"};
    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
#else
    const char *extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
#endif
    uint32_t layerCount;
    uint32_t extCount;
    VkLayerProperties *layerProps;
    VkExtensionProperties *extProps;

    VkApplicationInfo appInfo;
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 4, 341);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "Reaction Game";
    appInfo.pEngineName = "Engine";

    VkInstanceCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
    info.ppEnabledExtensionNames = extensions;
#ifdef DEBUG
    info.enabledLayerCount = 1;
    info.ppEnabledLayerNames = layers;
#else
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = NULL;
#endif

    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    layerProps = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProps);
    for(uint32_t i = 0; i < layerCount; i++)
    {
        __android_log_print(ANDROID_LOG_FATAL, "Vulkan","Layer %d.): %s", i, layerProps[i].layerName);
    }
    free(layerProps);

    vkEnumerateInstanceExtensionProperties(NULL, &extCount, NULL);
    extProps = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extCount, extProps);
    for(uint32_t i = 0; i < extCount; i++)
    {
        __android_log_print(ANDROID_LOG_FATAL, "Vulkan","Extension %d.): %s", i, extProps[i].extensionName);
    }
    free(extProps);

    return vkCreateInstance(&info, NULL, &base->devBase.instance);
}

static VkResult createDevice(EngineBase *base)
{
    uint32_t i;
    uint32_t gpuCount;
    VkResult result;
    VkPhysicalDevice *devices;
    VkDeviceQueueCreateInfo queue_info;
    VkDeviceCreateInfo info;
    uint32_t queueFamilyCount;
    VkPhysicalDeviceFeatures features;
    VkQueueFamilyProperties *queue_props;
    uint32_t layerCount;
    VkLayerProperties *props;

    float prios[] = {1.0f};
    const char *extensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    const char *layers[1] = {"VK_LAYER_KHRONOS_validation"};

    vkEnumeratePhysicalDevices(base->devBase.instance, &gpuCount, NULL);
    devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpuCount);
    vkEnumeratePhysicalDevices(base->devBase.instance, &gpuCount, devices);

    base->devBase.gpu = devices[0];

    vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, NULL);
    queue_props = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, queue_props);

    for(i = 0; i < gpuCount; i++)
    {
        if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            base->queueFamilyIndex = i;
            break;
        }
    }

    vkGetPhysicalDeviceFeatures(base->devBase.gpu, &features);

    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = prios;
    queue_info.queueFamilyIndex = base->queueFamilyIndex;
    queue_info.pNext = NULL;
    queue_info.flags = 0;

    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
#ifdef DEBUG
    info.enabledLayerCount = 1;
    info.ppEnabledLayerNames = layers;
#else
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = NULL;
#endif
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue_info;
    info.pEnabledFeatures = &features;

    result = vkCreateDevice(devices[0], &info, NULL, &base->devBase.device);
    if(result == VK_SUCCESS)
    {
        vkGetDeviceQueue(base->devBase.device, base->queueFamilyIndex, 0, &base->queue);

        vkEnumerateDeviceLayerProperties(base->devBase.gpu, &layerCount, NULL);
        props = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layerCount);
        vkEnumerateDeviceLayerProperties(base->devBase.gpu, &layerCount, props);

        for(i = 0; i < layerCount; i++)
        {
            __android_log_print(ANDROID_LOG_FATAL, "Vulkan","Layer %d.): %s", i, props[i].layerName);
        }
        free(props);
    }
    return result;
}

static VkResult createSurface(EngineBase *base, ANativeWindow *wnd)
{
    VkAndroidSurfaceCreateInfoKHR info;
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    info.flags = 0;
    info.pNext = NULL;
    info.window = wnd;

    return vkCreateAndroidSurfaceKHR(base->devBase.instance, &info, NULL, &base->devBase.surface);
}

static VkResult createSwapchain(EngineBase *base, uint32_t width, uint32_t height)
{
    uint32_t i;
    VkResult result;
    VkSurfaceCapabilitiesKHR surfaceCaps;
    VkImage *swapchainImages;

    VkExtent2D extent;
    extent.width = width;
    extent.height = height;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(base->devBase.gpu, base->devBase.surface, &surfaceCaps);

    VkSwapchainCreateInfoKHR info;
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.flags = 0;
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    info.imageArrayLayers = 1;
    info.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    info.imageExtent = extent;
    info.clipped = VK_FALSE;
    info.minImageCount = surfaceCaps.minImageCount;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    info.surface = base->devBase.surface;
    info.pNext = NULL;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &base->queueFamilyIndex;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(base->devBase.device, &info, NULL, &base->swapchain);

    if(result == VK_SUCCESS)
    {
        vkGetSwapchainImagesKHR(base->devBase.device, base->swapchain, &base->imageCount, NULL);
        swapchainImages =(VkImage *)malloc(sizeof(VkImage) * base->imageCount);
        base->renderImage = (RenderImage *)malloc(sizeof(RenderImage) * base->imageCount);
        vkGetSwapchainImagesKHR(base->devBase.device, base->swapchain, &base->imageCount, swapchainImages);
        for(i = 0; i < base->imageCount; i++)
        {
            base->renderImage[i].colorImage = swapchainImages[i];
        }
        free(swapchainImages);
    }

    return result;
}

static VkResult createImageViews(EngineBase *base, uint32_t index)
{
    VkResult result;
    VkImageViewCreateInfo color_info;
    color_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_info.flags = 0;
    color_info.pNext = NULL;
    color_info.image = base->renderImage[index].colorImage;
    color_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    color_info.components.r = VK_COMPONENT_SWIZZLE_R;
    color_info.components.g = VK_COMPONENT_SWIZZLE_G;
    color_info.components.b = VK_COMPONENT_SWIZZLE_B;
    color_info.components.a = VK_COMPONENT_SWIZZLE_A;
    color_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_info.subresourceRange.baseArrayLayer = 0;
    color_info.subresourceRange.baseMipLevel = 0;
    color_info.subresourceRange.levelCount = 1;
    color_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(base->devBase.device, &color_info, NULL, &base->renderImage[index].colorView);

    if(result == VK_SUCCESS)
    {
        VkImageViewCreateInfo depth_info;
        depth_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depth_info.image = base->renderImage[index].depthImage;
        depth_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depth_info.flags = 0;
        depth_info.pNext = NULL;
        depth_info.format = VK_FORMAT_D32_SFLOAT;
        depth_info.components.r = VK_COMPONENT_SWIZZLE_R;
        depth_info.components.g = VK_COMPONENT_SWIZZLE_G;
        depth_info.components.b = VK_COMPONENT_SWIZZLE_B;
        depth_info.components.a = VK_COMPONENT_SWIZZLE_A;
        depth_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth_info.subresourceRange.baseArrayLayer = 0;
        depth_info.subresourceRange.baseMipLevel = 0;
        depth_info.subresourceRange.levelCount = 1;
        depth_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(base->devBase.device, &depth_info, NULL, &base->renderImage[index].depthView);
    }
    return result;
}

static VkResult createFramebuffer(EngineBase *base, uint32_t width, uint32_t height, uint32_t index)
{
    VkImageView attachments[2] = {base->renderImage[index].colorView, base->renderImage[index].depthView};

    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = base->renderImage[index].renderPass;
    info.width = width;
    info.height = height;
    info.layers = 1;
    info.attachmentCount = 2;
    info.pAttachments = attachments;
    info.flags = 0;
    info.pNext = NULL;

    return vkCreateFramebuffer(base->devBase.device, &info, NULL, &base->renderImage[index].frameBuffer);
}

static VkResult createRenderpass(EngineBase *base, uint32_t index)
{
    VkAttachmentDescription attachments[2];
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].flags = 0;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].flags = 0;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkAttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef;
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency depens;
    depens.dependencyFlags = 0;
    depens.srcSubpass = VK_SUBPASS_EXTERNAL;
    depens.srcAccessMask = 0;
    depens.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    depens.dstSubpass = 0;
    depens.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    depens.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 2;
    info.pAttachments = attachments;
    info.dependencyCount = 1;
    info.pDependencies = &depens;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.flags = 0;
    info.pNext = NULL;

    return vkCreateRenderPass(base->devBase.device, &info, NULL, &base->renderImage[index].renderPass);
}

static VkResult createCommandPool(EngineBase *base)
{
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = NULL;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = base->queueFamilyIndex;

    return vkCreateCommandPool(base->devBase.device, &info, NULL, &base->renderRes.cmdPool);
}

static VkResult createDescriptorPool(EngineBase *base)
{
    VkDescriptorPoolSize uniforms[2];
    uniforms[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniforms[0].descriptorCount = 1;

    uniforms[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    uniforms[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = 2;
    info.pPoolSizes = uniforms;
    info.flags = 0;
    info.maxSets = 1;

    return vkCreateDescriptorPool(base->devBase.device, &info, NULL, &base->renderRes.descrPool);
}

void setCurrentScene(RenderScene *scene)
{
    currentScene = scene;
}

void *mainLoop(void* args)
{
    uint32_t imgIndex = 0;
    EngineBase *base = (EngineBase*)args;

    VkResult result;
    VkSemaphoreCreateInfo semaphoreInfo[2];
    semaphoreInfo[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo[0].pNext = NULL;
    semaphoreInfo[0].flags = 0;

    semaphoreInfo[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo[1].pNext = NULL;
    semaphoreInfo[1].flags = 0;

    VkSemaphore acquire;
    VkSemaphore submit;
    result = vkCreateSemaphore(base->devBase.device, &semaphoreInfo[0], NULL, &acquire);
    __android_log_print(ANDROID_LOG_FATAL, "VULKAN", "Senmaphore Creation: %d", result);
    result = vkCreateSemaphore(base->devBase.device, &semaphoreInfo[1], NULL, &submit);
    __android_log_print(ANDROID_LOG_FATAL, "VULKAN", "Senmaphore Creation: %d", result);

    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentScene->pipeline.cmdBuffers[imgIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &submit;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &acquire;
    submitInfo.pWaitDstStageMask = &waitStages;

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &submit;
    presentInfo.swapchainCount = 1;
    presentInfo.pResults = &result;
    presentInfo.pSwapchains = &base->swapchain;

    while(true)
    {
        vkAcquireNextImageKHR(base->devBase.device, base->swapchain, UINT64_MAX, acquire, VK_NULL_HANDLE, &imgIndex);
        submitInfo.pCommandBuffers = &currentScene->pipeline.cmdBuffers[imgIndex];
        vkQueueSubmit(base->queue, 1, &submitInfo,VK_NULL_HANDLE);

        presentInfo.pImageIndices = &imgIndex;
        vkQueuePresentKHR(base->queue, &presentInfo);
    }
    return NULL;
}
