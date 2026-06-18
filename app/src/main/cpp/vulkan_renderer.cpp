//
// Created by lacko on 3/12/26.
// kkOJPp7gbvVocAL
//
#include "vulkan_renderer.h"

#include <android/log.h>
#include <android/window.h>
#include <string.h>
#include <math.h>

bool running = true;
RenderScene *currentScene;
static VkFormat swapchainFormat;

static void createDebugMessenger(EngineBase *base);

static VkResult createInstance(EngineBase *base);
static VkResult createDevice(EngineBase *base);
static VkResult createSurface(EngineBase *base, struct ANativeWindow *wnd);
static VkResult createSwapchain(EngineBase *base, uint32_t width, uint32_t height);

static VkResult createImageViews(EngineBase *base, uint32_t index);
static VkResult createDepthView(EngineBase *base);
static VkResult createDepthImage(EngineBase *base, uint32_t width, uint32_t height);
static VkResult createFramebuffer(EngineBase *base, uint32_t width, uint32_t height, uint32_t index);
static VkResult createRenderpass(EngineBase *base, uint32_t index);
static VkResult createCommandPool(EngineBase *base);
static VkResult createDescriptorPool(EngineBase *base, RenderScene *scene);

static float viewMat[16] =
    {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 3.0, 1.0
    };

VkDebugUtilsMessengerEXT messenger;
//#define DEBUG
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
    CHECK_RESULT(createDepthImage(base, width, height));
    CHECK_RESULT(createDepthView(base));
    for(i = 0; i < base->imageCount; i++)
    {
        CHECK_RESULT(createImageViews(base, i));
        CHECK_RESULT(createRenderpass(base, i));
        CHECK_RESULT(createFramebuffer(base, width, height, i));
    }
}

void makeDescriptorPool(EngineBase *base, RenderScene *scene)
{
    CHECK_RESULT(createDescriptorPool(base, scene));
}

void releaseSurfaceImages(EngineBase *base)
{
    vkDestroyImageView(base->devBase.device, base->depth.depthView, NULL);
    vkFreeMemory(base->devBase.device, base->depth.depthMemory, NULL);
    vkDestroyImage(base->devBase.device, base->depth.depthImage, NULL);
    for(uint32_t i = 0; i < base->imageCount; i++)
    {
        vkDestroyFramebuffer(base->devBase.device, base->renderImage[i].frameBuffer, NULL);
        vkDestroyRenderPass(base->devBase.device, base->renderImage[i].renderPass, NULL);
        vkDestroyImageView(base->devBase.device, base->renderImage[i].colorView, NULL);
    }
    vkDestroySwapchainKHR(base->devBase.device, base->swapchain, NULL);
    vkDestroySurfaceKHR(base->devBase.instance, base->devBase.surface, NULL);
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
    VkQueueFamilyProperties *queueProps;
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
    queueProps = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, queueProps);

    for(i = 0; i < queueFamilyCount; i++)
    {
        if((queueProps[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT)) != 0)
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
    free(queueProps);
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
    uint32_t formatCount;
    VkResult result;
    VkSurfaceCapabilitiesKHR surfaceCaps;
    VkImage *swapchainImages;
    VkSurfaceFormatKHR *formats;

    VkExtent2D extent;
    extent.width = width;
    extent.height = height;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(base->devBase.gpu, base->devBase.surface, &surfaceCaps);
    vkGetPhysicalDeviceSurfaceFormatsKHR(base->devBase.gpu, base->devBase.surface, &formatCount, NULL);
    formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR)*formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(base->devBase.gpu, base->devBase.surface, &formatCount, formats);

    swapchainFormat = formats[0].format;

    VkSwapchainCreateInfoKHR info;
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    info.flags = 0;
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.imageColorSpace = formats[0].colorSpace;
    info.imageArrayLayers = 1;
    info.imageFormat = formats[0].format;
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
        free(formats);
    }

    return result;
}

static VkResult createDepthImage(EngineBase *base, uint32_t width, uint32_t height)
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

    result = vkCreateImage(base->devBase.device, &info, NULL, &base->depth.depthImage);

    if(result == VK_SUCCESS)
    {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(base->devBase.device, base->depth.depthImage, &memReqs);

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
            base->depth.depthMemory = memory;
            result = vkBindImageMemory(base->devBase.device, base->depth.depthImage, memory, 0);
        }
    }
    return result;
}

static VkResult createDepthView(EngineBase *base)
{
    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkComponentMapping component;
    component.r = VK_COMPONENT_SWIZZLE_R;
    component.g = VK_COMPONENT_SWIZZLE_G;
    component.b = VK_COMPONENT_SWIZZLE_B;
    component.a = VK_COMPONENT_SWIZZLE_A;

    VkImageViewCreateInfo depthInfo;
    depthInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthInfo.pNext = NULL;
    depthInfo.flags = 0;
    depthInfo.image = base->depth.depthImage;
    depthInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthInfo.format = VK_FORMAT_D32_SFLOAT;
    depthInfo.components = component;
    depthInfo.subresourceRange = subresourceRange;

    return vkCreateImageView(base->devBase.device, &depthInfo, NULL, &base->depth.depthView);
}

static VkResult createImageViews(EngineBase *base, uint32_t index)
{
    VkResult result;

    VkComponentMapping component;
    component.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    component.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    component.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    component.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    VkImageViewCreateInfo colorInfo;
    colorInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorInfo.pNext = NULL;
    colorInfo.flags = 0;
    colorInfo.image = base->renderImage[index].colorImage;
    colorInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorInfo.format = swapchainFormat;
    colorInfo.components = component;
    colorInfo.subresourceRange = subresourceRange;

    result = vkCreateImageView(base->devBase.device, &colorInfo, NULL, &base->renderImage[index].colorView);

    if(result == VK_SUCCESS)
    {

    }
    return result;
}

static VkResult createFramebuffer(EngineBase *base, uint32_t width, uint32_t height, uint32_t index)
{
    VkImageView attachments[2] = {base->renderImage[index].colorView, base->depth.depthView};

    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.renderPass = base->renderImage[index].renderPass;
    info.attachmentCount = 2;
    info.pAttachments = attachments;
    info.width = width;
    info.height = height;
    info.layers = 1;

    return vkCreateFramebuffer(base->devBase.device, &info, NULL, &base->renderImage[index].frameBuffer);
}

static VkResult createRenderpass(EngineBase *base, uint32_t index)
{
    VkAttachmentDescription attachments[2];
    attachments[0].flags = 0;
    attachments[0].format = swapchainFormat;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].flags = 0;
    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef;
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDependency depens;
    depens.dependencyFlags = 0;
    depens.srcSubpass = VK_SUBPASS_EXTERNAL;
    depens.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depens.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;;
    depens.dstSubpass = 0;
    depens.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depens.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthRef;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;

    VkRenderPassCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.attachmentCount = 2;
    info.pAttachments = attachments;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &depens;

    return vkCreateRenderPass(base->devBase.device, &info, NULL, &base->renderImage[index].renderPass);
}

static VkResult createCommandPool(EngineBase *base)
{
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = NULL;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = base->queueFamilyIndex;

    return vkCreateCommandPool(base->devBase.device, &info, NULL, &base->cmdPool);
}

static VkResult createDescriptorPool(EngineBase *base, RenderScene *scene)
{
    VkDescriptorPoolSize uniforms[2];
    uniforms[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniforms[0].descriptorCount = 1;

    uniforms[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniforms[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = 2;
    info.pPoolSizes = uniforms;
    info.flags = 0;
    info.maxSets = 2;

    return vkCreateDescriptorPool(base->devBase.device, &info, NULL, &scene->pipeline.descrPool);
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

    VkDescriptorBufferInfo writeBuffers[2];
    writeBuffers[0].offset = 0;
    writeBuffers[0].range = 64;
    writeBuffers[0].buffer = currentScene->memory.uniformBuffers[0];

    VkWriteDescriptorSet write[2];
    write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[0].pNext = NULL;
    write[0].descriptorCount = 1;
    write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[0].pBufferInfo = &writeBuffers[0];
    write[0].dstArrayElement = 0;
    write[0].dstBinding = 0;
    write[0].dstSet = currentScene->pipeline.descriptors[0];
    write[0].pImageInfo = NULL;
    write[0].pTexelBufferView = NULL;

    writeBuffers[1].offset = 0;
    writeBuffers[1].range = 12;
    writeBuffers[1].buffer = currentScene->memory.uniformBuffers[1];

    write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[1].pNext = NULL;
    write[1].descriptorCount = 1;
    write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[1].pBufferInfo = &writeBuffers[1];
    write[1].dstArrayElement = 0;
    write[1].dstBinding = 1;
    write[1].dstSet = currentScene->pipeline.descriptors[1];
    write[1].pImageInfo = NULL;
    write[1].pTexelBufferView = NULL;
    __android_log_print(ANDROID_LOG_FATAL, "VULKAN", "%X", currentScene->pipeline.descriptors[0]);
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

    void *vertexUniforms, *fragmentUniforms;

    float alpha = 0.0, cosa = 1.0, sina = 0.0;
    float lightPos[3] = {10.0, 10.0, 0.0};

    vkMapMemory(base->devBase.device, currentScene->memory.uniformBufferMemories[1], (VkDeviceSize)0, (VkDeviceSize)12, 0, &fragmentUniforms);
    memcpy(fragmentUniforms, viewMat, 12);
    vkUnmapMemory(base->devBase.device, currentScene->memory.uniformBufferMemories[1]);

    while(running)
    {
        alpha += 0.02f;
        cosa = cos(alpha);
        sina = sin(alpha);

        viewMat[5]  = cosa;
        viewMat[6]  = sina;
        viewMat[9]  = -sina;
        viewMat[10] = cosa;
        vkMapMemory(base->devBase.device, currentScene->memory.uniformBufferMemories[0], (VkDeviceSize)0, (VkDeviceSize)64, 0, &vertexUniforms);
        memcpy(vertexUniforms, viewMat, 64);
        vkUnmapMemory(base->devBase.device, currentScene->memory.uniformBufferMemories[0]);

        vkUpdateDescriptorSets(base->devBase.device, 1, write, 0, NULL);

        vkAcquireNextImageKHR(base->devBase.device, base->swapchain, UINT64_MAX, acquire, VK_NULL_HANDLE, &imgIndex);
        submitInfo.pCommandBuffers = &currentScene->pipeline.cmdBuffers[imgIndex];
        vkQueueSubmit(base->queue, 1, &submitInfo,VK_NULL_HANDLE);

        presentInfo.pImageIndices = &imgIndex;
        __android_log_print(ANDROID_LOG_FATAL, "VULKAN", "%d", imgIndex);
        vkQueuePresentKHR(base->queue, &presentInfo);
    }
    return NULL;
}
