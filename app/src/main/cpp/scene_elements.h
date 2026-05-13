//
// Created by laszlo on 4/1/26.
//

#ifndef REACTIONGAME_SCENE_ELEMENTS_H
#define REACTIONGAME_SCENE_ELEMENTS_H


#include <vulkan/vulkan.h>
#include "vulkan_renderer.h"

void makeTexture(EngineBase *base, RenderScene *scene, uint32_t textureCount, TextureInfo *info);
void makeVertexBuffer(EngineBase *base, RenderScene *scene, uint32_t index, float *vertexData, uint32_t floatCount);


#endif //REACTIONGAME_SCENE_ELEMENTS_H
