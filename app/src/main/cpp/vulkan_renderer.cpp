//
// Created by lacko on 3/12/26.
//
#include "vulkan_renderer.h"

#include <android/log.h>
#include <android/window.h>

static uint32_t queueFamilyIndex;

static VkResult createInstance(EngineBase *base);
static VkResult createDevice(EngineBase *base);
static VkResult createSurface(EngineBase *base, struct ANativeWindow *wnd);
static VkResult createSwapchain(EngineBase *base, uint32_t width, uint32_t height);
static VkResult createImageViews(EngineBase *base, uint32_t index);
static VkResult createDepthImage(EngineBase *base, uint32_t width, uint32_t height, uint32_t index);

static VkResult createFramebuffer(EngineBase *base, uint32_t width, uint32_t height, uint32_t index);
static VkResult createRenderpass(EngineBase *base, uint32_t index);

#define CHECK_RESULT(RESULT)        \
    do {                            \
        if((RESULT) != VK_SUCCESS)  \
        {                           \
           __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "%s:%d, ERROR:%d fn:%s",__FILE__, __LINE__, RESULT, #RESULT); \
           return;                  \
        }                           \
    } while(0)

void makeEngineBase(EngineBase *base)
{
    __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "Assert OK.");
    CHECK_RESULT(createInstance(base));
    CHECK_RESULT(createDevice(base));
}

void makeRenderImage(EngineBase *base, uint32_t width, uint32_t height)
{
    uint32_t i;
    __android_log_print(ANDROID_LOG_FATAL, "Vulkan", "Assert OK.");
    CHECK_RESULT(createSwapchain(base, width, height));
    for(i = 0; i < 2; i++)
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
    info.pQueueFamilyIndices = &queueFamilyIndex;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    info.mipLevels = 1;
    info.format = VK_FORMAT_D32_SFLOAT;
    info.arrayLayers = 1;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    result = vkCreateImage(base->device, &info, NULL, &base->renderImage[index].depthImage);

    if(result == VK_SUCCESS)
    {
        VkMemoryRequirements memReqs;
        vkGetImageMemoryRequirements(base->device, base->renderImage[index].depthImage, &memReqs);

        VkPhysicalDeviceMemoryProperties props;
        vkGetPhysicalDeviceMemoryProperties(base->gpu, &props);

        for (i = 0; i < props.memoryTypeCount; i++) {
            heapIndex = props.memoryTypes[i].heapIndex;
            if ((props.memoryHeaps[heapIndex].flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0) {
                break;
            }
        }

        VkMemoryAllocateInfo memInfo;
        memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memInfo.allocationSize = memReqs.size;
        memInfo.pNext = NULL;
        memInfo.memoryTypeIndex = i;

        result = vkAllocateMemory(base->device, &memInfo, NULL, &memory);

        if(result == VK_SUCCESS)
        {
            result = vkBindImageMemory(base->device, base->renderImage->depthImage, memory, 0);
        }
    }
    return result;
}

void makeSurface(EngineBase *base, struct ANativeWindow *wnd)
{
    createSurface(base, wnd);
}

static VkResult createInstance(EngineBase *base)
{
    const char *extensions = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
    VkApplicationInfo app_info;
    app_info.applicationVersion = VK_MAKE_VERSION(1,0,0);
    app_info.apiVersion = VK_MAKE_VERSION(1, 4, 341);
    app_info.engineVersion = VK_MAKE_VERSION(1, 0,0);
    app_info.pApplicationName = "Reaction Game";
    app_info.pEngineName = "Engine";

    VkInstanceCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.pApplicationInfo = &app_info;
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = &extensions;
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = NULL;

    return vkCreateInstance(&info, NULL, &base->instance);
}

static VkResult createDevice(EngineBase *base)
{
    uint32_t i;
    uint32_t gpuCount;
    VkPhysicalDevice *devices;
    VkDeviceQueueCreateInfo queue_info;
    VkDeviceCreateInfo info;
    uint32_t queueFamilyCount;
    VkPhysicalDeviceFeatures features;
    VkQueueFamilyProperties *queue_props;

    float prios[] = {1.0f};
    const char *extensions[1] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vkEnumeratePhysicalDevices(base->instance, &gpuCount, NULL);
    devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpuCount);
    vkEnumeratePhysicalDevices(base->instance, &gpuCount, devices);

    base->gpu = devices[0];

    vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, NULL);
    queue_props = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(devices[0], &queueFamilyCount, queue_props);

    for(i = 0; i < gpuCount; i++)
    {
        if((queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            queueFamilyIndex = i;
            break;
        }
    }

    vkGetPhysicalDeviceFeatures(base->gpu, &features);

    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = prios;
    queue_info.queueFamilyIndex = queueFamilyIndex;
    queue_info.pNext = NULL;
    queue_info.flags = 0;

    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.flags = 0;
    info.pNext = NULL;
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = NULL;
    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue_info;
    info.pEnabledFeatures = &features;

    return vkCreateDevice(devices[0], &info, NULL, &base->device);
}

static VkResult createSurface(EngineBase *base, struct ANativeWindow *wnd)
{
    VkAndroidSurfaceCreateInfoKHR info;
    info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    info.flags = 0;
    info.window = wnd;
    info.pNext = NULL;

    return vkCreateAndroidSurfaceKHR(base->instance, &info, NULL, &base->surface);
}

static VkResult createSwapchain(EngineBase *base, uint32_t width, uint32_t height)
{
    uint32_t i;
    uint32_t imageCount = 2;
    VkResult result;
    VkImage *swapchainImages;
    VkExtent2D extent;
    extent.width = width;
    extent.height = height;

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
    info.minImageCount = imageCount;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    info.surface = base->surface;
    info.pNext = NULL;
    info.queueFamilyIndexCount = 1;
    info.pQueueFamilyIndices = &queueFamilyIndex;
    info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    info.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR(base->device, &info, NULL, &base->swapchain);

    if(result == VK_SUCCESS)
    {
        vkGetSwapchainImagesKHR(base->device, base->swapchain, &base->imageCount, NULL);
        swapchainImages =(VkImage *)malloc(sizeof(VkImage) * base->imageCount);
        base->renderImage = (RenderImage *)malloc(sizeof(RenderImage) * base->imageCount);
        vkGetSwapchainImagesKHR(base->device, base->swapchain, &base->imageCount, swapchainImages);
        for(i = 0; i < imageCount; i++)
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
    color_info.image = base->renderImage[index].colorImage;
    color_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_info.flags = 0;
    color_info.pNext = NULL;
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

    result = vkCreateImageView(base->device, &color_info, NULL, &base->renderImage[index].colorView);

    if(result == VK_SUCCESS)
    {
        VkImageViewCreateInfo depth_info;
        depth_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depth_info.image = base->renderImage[index].depthImage;
        depth_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depth_info.flags = 0;
        depth_info.pNext = NULL;
        depth_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        depth_info.components.r = VK_COMPONENT_SWIZZLE_R;
        depth_info.components.g = VK_COMPONENT_SWIZZLE_G;
        depth_info.components.b = VK_COMPONENT_SWIZZLE_B;
        depth_info.components.a = VK_COMPONENT_SWIZZLE_A;
        depth_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        depth_info.subresourceRange.baseArrayLayer = 0;
        depth_info.subresourceRange.baseMipLevel = 0;
        depth_info.subresourceRange.levelCount = 1;
        depth_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(base->device, &depth_info, NULL, &base->renderImage[index].depthView);
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

    return vkCreateFramebuffer(base->device, &info, NULL, &base->renderImage[index].frameBuffer);
}

static VkResult createRenderpass(EngineBase *base, uint32_t index)
{
    VkAttachmentDescription attachments[2];
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].flags = 0;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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
    colorRef.attachment = 1;
    colorRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass;
    subpass.flags = 0;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = NULL;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = NULL;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_BEGIN_RANGE;
    subpass.pResolveAttachments = NULL;
    subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 2;
    info.pAttachments = attachments;
    info.dependencyCount = 0;
    info.pDependencies = NULL;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.flags = 0;
    info.pNext = NULL;

    return vkCreateRenderPass(base->device, &info, NULL, &base->renderImage[index].renderPass);
}

VkResult createCommandPool(EngineBase *base, RenderRes *resource)
{
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = NULL;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = queueFamilyIndex;

    return vkCreateCommandPool(base->device, &info, NULL, &resource->cmdPool);
}

VkResult createDescriptorPool(EngineBase *base, RenderRes *resource)
{
    VkDescriptorPoolSize uniforms[2];
    uniforms[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniforms[0].descriptorCount = 1;

    uniforms[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    uniforms[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = 1;
    info.pPoolSizes = uniforms;
    info.flags = 0;
    info.maxSets = 1;

    return vkCreateDescriptorPool(base->device, &info, NULL, &resource->descrPool);
}
