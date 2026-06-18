//
// Created by laszlo on 4/1/26.
//

#ifndef REACTIONGAME_SCENE_ELEMENTS_H
#define REACTIONGAME_SCENE_ELEMENTS_H


#include <vulkan/vulkan.h>
#include "vulkan_renderer.h"

void makeTexture(EngineBase *base, RenderScene *scene, uint32_t textureCount, TextureInfo *info);
void makeVertexBuffer(EngineBase *base, RenderScene *scene, uint32_t index, float *vertexData, float *instanceData, uint32_t vertexFloatCount, uint32_t instanceFloatCount);
void makeUniformBuffer(EngineBase *base, RenderScene *scene, uint32_t floatCount, uint32_t index);
void makeCommandBuffers(EngineBase *base, RenderScene *scene);
void makeDescriptorSets(EngineBase *base, RenderScene *scene);


#endif //REACTIONGAME_SCENE_ELEMENTS_H
