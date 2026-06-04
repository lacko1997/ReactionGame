//
// Created by laszlo on 5/11/26.
//

#ifndef REACTIONGAME_PIPELINES_H
#define REACTIONGAME_PIPELINES_H

#include <vulkan/vulkan.h>
#include "vulkan_renderer.h"

#define MODEL_SHADER_MODULE_INDEX 0
#define TEXT_SHADER_MODULE_INDEX 1

void initCodesArray();
void putSpvCode(uint32_t index, char* byteArray, uint32_t byteCount);

void makeModelPipeline(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height);
void makeTextPipeline(EngineBase *base, RenderScene *scene);

void releasePipeline(EngineBase *base, RenderScene *scene);

#endif //REACTIONGAME_PIPELINES_H
