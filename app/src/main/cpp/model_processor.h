//
// Created by laszlo on 4/28/26.
//

#ifndef REACTIONGAME_MODEL_PROCESSOR_H
#define REACTIONGAME_MODEL_PROCESSOR_H

#include <stdint.h>

void beginModel(uint16_t vertexCount, uint16_t uvCount, uint16_t normalCount, uint16_t triangleCount);

void processPositionLine(char *line, uint32_t len);
void processTextureLine(char *line, uint32_t len);
void processNormalLine(char *line, uint32_t len);
void processTriangleLine(char *line, uint32_t len);

#endif //REACTIONGAME_MODEL_PROCESSOR_H
