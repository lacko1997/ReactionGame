//
// Created by laszlo on 6/11/26.
//

#include "main_menu.h"
#include "game_area.h"
#include "model_processor.h"
#include "scene_elements.h"
#include "pipelines.h"

static RenderScene mainMenuScene;
static RenderScene gameAreaScene;

static void acquireGameAreaUniforms(EngineBase *base)
{
    makeUniformBuffer(base, &gameAreaScene, 16, 0);
    makeUniformBuffer(base, &gameAreaScene, 3, 1);
}

void makeScenes(EngineBase *base, uint32_t width, uint32_t height)
{
    //makeMainMenu(base, &mainMenuScene, width, height);

    makeModelPipeline(base, &gameAreaScene, width, height);
    makeDescriptorPool(base, &gameAreaScene);
    makeDescriptorSets(base, &gameAreaScene);

    makeVulkanBuffers(base, &gameAreaScene);
    acquireGameAreaUniforms(base);
    makeGameArea(base, &gameAreaScene, width, height);
}

void sceneManager_allocateSceneMemory(uint32_t sceneIndex, uint32_t modelCount, uint32_t uniformBufferCount)
{
    gameAreaScene.memory.modelCount = modelCount;
    gameAreaScene.memory.modelBuffers = (Model*)malloc(sizeof(Model) * modelCount);
    gameAreaScene.memory.uniformBuffers  = (VkBuffer*)malloc(sizeof(VkBuffer) * uniformBufferCount);
    gameAreaScene.memory.uniformBufferMemories = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * uniformBufferCount);
}

void sceneMamager_releaseScenes(EngineBase *base)
{
    releasePipeline(base, &gameAreaScene);
}

RenderScene *sceneManager_getScene(uint32_t sceneIndex)
{
    return &gameAreaScene;
}
