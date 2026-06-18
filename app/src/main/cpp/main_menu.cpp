//
// Created by lacko on 3/28/26.
//

#include "main_menu.h"

#include <string.h>
#include <android/log.h>
#include "scene_elements.h"


static VkResult recordCommandBuffer(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height);

void makeMainMenu(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    makeCommandBuffers(base, scene);
    CHECK_RESULT(recordCommandBuffer(base, scene, width, height));
}

void resumeMainMenu(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    for(uint32_t i = 0; i < base->imageCount; i++)
    {
        vkResetCommandBuffer(scene->pipeline.cmdBuffers[i], 0);
    }
    CHECK_RESULT(recordCommandBuffer(base, scene, width, height));
}

static VkResult recordCommandBuffer(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height)
{
    VkResult result = VK_SUCCESS;
    VkRect2D area;
    area.offset.x = 0;
    area.offset.y = 0;
    area.extent.width = width;
    area.extent.height = height;

    VkClearRect rect;
    rect.baseArrayLayer = 0;
    rect.layerCount = 1;
    rect.rect = area;

    VkClearValue clearVals[2];
    memset(clearVals[0].color.float32, 0, sizeof(clearVals[0].color.float32));
    clearVals[0].color.float32[2] = 0.0;
    clearVals[0].color.float32[3] = 1.0;
    clearVals[1].depthStencil.depth = 1.0f;
    clearVals[1].depthStencil.stencil = 0;

    VkCommandBufferBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    info.pNext = NULL;
    info.pInheritanceInfo = NULL;

    VkDeviceSize offset = 0;

    for(uint32_t i = 0; i < base->imageCount && result == VK_SUCCESS; i++)
    {
        VkRenderPassBeginInfo rpBegInfo;
        rpBegInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegInfo.pNext = NULL;
        rpBegInfo.renderPass = base->renderImage[i].renderPass;
        rpBegInfo.framebuffer = base->renderImage[i].frameBuffer;
        rpBegInfo.renderArea = area;
        rpBegInfo.clearValueCount = 2;
        rpBegInfo.pClearValues = clearVals;

        result = vkBeginCommandBuffer(scene->pipeline.cmdBuffers[i], &info);
        vkCmdBeginRenderPass(scene->pipeline.cmdBuffers[i], &rpBegInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(scene->pipeline.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, scene->pipeline.pipeline[i]);
        vkCmdBindVertexBuffers(scene->pipeline.cmdBuffers[i], 0, 1, &scene->memory.modelBuffers[0].vertexBuffer, &offset);
        vkCmdBindDescriptorSets(scene->pipeline.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, scene->pipeline.pipelineLayout, 0, 1,scene->pipeline.descriptors, 0, NULL);
        //vkCmdPushConstants(scene->pipeline.cmdBuffers[i], scene->pipeline.pipelineLayout, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, sizeof(persp), persp);
        vkCmdDraw(scene->pipeline.cmdBuffers[i], scene->memory.modelBuffers[0].vertexCount, 1, 0, 0);
        vkCmdEndRenderPass(scene->pipeline.cmdBuffers[i]);
        vkEndCommandBuffer(scene->pipeline.cmdBuffers[i]);
    }
    return result;
}