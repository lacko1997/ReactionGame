//
// Created by laszlo on 6/27/26.
//

#ifndef REACTIONGAME_GAME_PARAMS_H
#define REACTIONGAME_GAME_PARAMS_H

#include <stdint.h>

typedef enum ButtonArrangement
{
    E_ARRANGEMENT_RECTANGULAR = 0,
    E_ARRANGEMENT_CIRCULAR = 1
}ButtonArrangement;

float *makeInstanceData(uint32_t *instanceDataFloatCount, uint32_t modelIndex);
float *generateGameTableData(uint32_t *vertexCount);

uint32_t getButtonCount();
void setButtonCount(uint32_t count);

#endif //REACTIONGAME_GAME_PARAMS_H
