//
// Created by laszlo on 6/11/26.
//

#ifndef REACTIONGAME_SCENE_MANAGER_H
#define REACTIONGAME_SCENE_MANAGER_H

#include "scene_elements.h"

void makeScenes(EngineBase *base, uint32_t width, uint32_t height);
void sceneManager_allocateSceneMemory(uint32_t sceneIndex, uint32_t modelCount, uint32_t uniformBufferCount);
RenderScene *sceneManager_getScene(uint32_t sceneIndex);
void sceneMamager_releaseScenes(EngineBase *base);

#endif //REACTIONGAME_SCENE_MANAGER_H
