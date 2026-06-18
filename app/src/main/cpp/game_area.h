//
// Created by laszlo on 6/10/26.
//

#ifndef REACTIONGAME_GAME_AREA_H
#define REACTIONGAME_GAME_AREA_H

#include <stdlib.h>

struct EngineBase;
struct RenderScene;

void makeGameArea(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height);
void resumeGameArea(EngineBase *base, RenderScene *scene, uint32_t width, uint32_t height);

#endif //REACTIONGAME_GAME_AREA_H
