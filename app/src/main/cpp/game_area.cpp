#include "game_area.h"

#include <string.h>
#include <math.h>
#include <android/log.h>

#include "scene_elements.h"

#define MAX_BUTTON_COUNT 9
float trasnformData[MAX_BUTTON_COUNT * 16];

uint32_t buttonCount = 3;

static VkResult recordCommandBuffer(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height);

float zNear = 1.0f;
float zFar = 100.0f;
float fovy = 3.14159265f / 4.0f;

float persp[32] =
        {
                1.0f / tan(fovy), 0.0, 0.0, 0.0,
                0.0, 1.0f / tan(fovy), 0.0, 0.0,
                0.0, 0.0, (zFar + zNear)/(zFar-zNear), zFar*zNear/(zFar-zNear),
                0.0, 0.0, -1.0, 0.0,
        };

void makeGameArea(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    makeCommandBuffers(base, scene);
    CHECK_RESULT(recordCommandBuffer(base, scene, width, height));
}

static VkResult recordCommandBuffer(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    VkResult result;
    float ratio = (float)width/(float)height;
    const int clearValueCount = 2;

    VkCommandBufferBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    info.pNext = NULL;
    info.pInheritanceInfo = NULL;

    VkRect2D area;
    area.offset.x = 0;
    area.offset.y = 0;
    area.extent.width = width;
    area.extent.height = height;

    VkClearValue clearVals[2];
    memset(clearVals[0].color.float32, 0, sizeof(clearVals[0].color.float32));
    clearVals[0].color.float32[3] = 1.0f;
    clearVals[1].depthStencil.depth = 1.0f;
    clearVals[1].depthStencil.stencil = 0;

    VkDeviceSize offsets[2] = {0, 0};
    VkBuffer buffers[2] = {scene->memory.modelBuffers[0].vertexBuffer, scene->memory.modelBuffers[0].instanceBuffer};
    persp[5] = ratio / tan(fovy);
    for(uint32_t i = 0; i < base->imageCount; i++)
    {
        VkRenderPassBeginInfo passInfo;
        passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        passInfo.pNext = NULL;
        passInfo.clearValueCount = 2;
        passInfo.pClearValues = clearVals;
        passInfo.renderArea = area;
        passInfo.renderPass = base->renderImage[i].renderPass;
        passInfo.framebuffer = base->renderImage[i].frameBuffer;

        result = vkBeginCommandBuffer(scene->pipeline.cmdBuffers[i], &info);
        vkCmdBeginRenderPass(scene->pipeline.cmdBuffers[i], &passInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(scene->pipeline.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, scene->pipeline.pipeline[i]);
        vkCmdBindVertexBuffers(scene->pipeline.cmdBuffers[i], 0, 1, buffers, offsets);
        vkCmdBindDescriptorSets(scene->pipeline.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, scene->pipeline.pipelineLayout, 0, 1, scene->pipeline.descriptors, 0,NULL);
        vkCmdPushConstants(scene->pipeline.cmdBuffers[i], scene->pipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(persp) , persp);
        vkCmdDraw(scene->pipeline.cmdBuffers[i], scene->memory.modelBuffers[0].vertexCount, 1, 0, 0);
        vkCmdEndRenderPass(scene->pipeline.cmdBuffers[i]);
        vkEndCommandBuffer(scene->pipeline.cmdBuffers[i]);
    }
    return result;
}
