//
// Created by laszlo on 4/28/26.
//

#ifndef REACTIONGAME_MODEL_PROCESSOR_H
#define REACTIONGAME_MODEL_PROCESSOR_H

#include <stdint.h>
#include "vulkan_renderer.h"

void beginModel(uint16_t positionCount, uint16_t uvCount, uint16_t normalCount, uint16_t vertexCount);

void processPositionLine(char *line, uint32_t len);
void processTextureLine(char *line, uint32_t len);
void processNormalLine(char *line, uint32_t len);
void processTriangleLine(char *line, uint32_t len);

void makeVulkanBuffers(EngineBase *base, RenderScene *scene);

void endModel();

#endif //REACTIONGAME_MODEL_PROCESSOR_H
