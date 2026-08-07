//
// Created by laszlo on 4/28/26.
//

#include "model_processor.h"

#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#include "scene_elements.h"
#include "game_params.h"

#define TRIANGLE_VERTEX_COUNT 3
#define VERTEX_FLOAT_COUNT_STRIDE 8

const uint32_t perTriangleFloatCount = TRIANGLE_VERTEX_COUNT * VERTEX_FLOAT_COUNT_STRIDE;

typedef struct RawModelIndices
{
    uint16_t *posIndex;
    uint16_t *texIndex;
    uint16_t *normIndex;
}RawModelIndices;

typedef struct RawModelData
{
    float *position;
    float *texture;
    float *normal;
}RawModelData;

typedef struct AtElement
{
    uint16_t atPos;
    uint16_t atTex;
    uint16_t atNorm;
    uint16_t atTriangle;
}AtElement;

typedef struct RawModel
{
    AtElement element;
    RawModelData data;
    RawModelIndices index;
}RawModel;

typedef struct RawModelList
{
    RawModel model;
    RawModelList *next;
}RawModelList;

static uint32_t modelCount = 0;
static RawModelList start;
static RawModelList *end = &start;

static float ident[16] =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

static void createDrawableModel(RawModel *model, float *bufferContent);

void beginModel(uint16_t positionCount, uint16_t uvCount, uint16_t normalCount, uint16_t vertexCount)
{
    end->model.data.position = (float*)malloc(sizeof(float) * 3 * positionCount);
    end->model.data.texture = (float*)malloc(sizeof(float) * 2 * uvCount);
    end->model.data.normal = (float*)malloc(sizeof(float) * 3 * normalCount);

    end->model.index.posIndex = (uint16_t*)malloc(sizeof(uint16_t) * vertexCount);
    end->model.index.texIndex = (uint16_t*)malloc(sizeof(uint16_t) * vertexCount);
    end->model.index.normIndex = (uint16_t*)malloc(sizeof(uint16_t) * vertexCount);

    end->model.element.atPos = 0;
    end->model.element.atTex = 0;
    end->model.element.atNorm = 0;
    end->model.element.atTriangle = 0;
}

void processPositionLine(char *line, uint32_t len)
{
    uint32_t at;
    char *token, *next;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if(line != NULL && len != 0)
    {
        line = &line[1];
        token = strtok_r(line, " ", &next);
        x = atof(token);
        token = strtok_r(next, " ", &next);
        y = atof(token);
        token = strtok_r(next, " ", &next);
        z = atof(token);
    }
    at = end->model.element.atPos;

    end->model.data.position[at * 3] = x;
    end->model.data.position[at * 3 + 1] = y;
    end->model.data.position[at * 3 + 2] = z;

    end->model.element.atPos++;
}

void processTextureLine(char *line, uint32_t len)
{
    uint32_t at;
    char *token, *next;
    float x = 0.0f, y = 0.0f;
    if(line != NULL && len != 0)
    {
        line = &line[2];
        token = strtok_r(line, " ", &next);
        x = atof(token);
        token = strtok_r(next, " ", &next);
        y = atof(token);
    }
    at = end->model.element.atTex;

    end->model.data.texture[at * 2] = x;
    end->model.data.texture[at * 2 + 1] = y;

    end->model.element.atTex++;
}

void processNormalLine(char *line, uint32_t len)
{
    uint32_t at;
    char *token, *next;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if(line != NULL && len != 0)
    {
        line = &line[2];
        token = strtok_r(line, " ", &next);
        x = atof(token);
        token = strtok_r(next, " ", &next);
        y = atof(token);
        token = strtok_r(next, " ", &next);
        z = atof(token);
    }
    at = end->model.element.atNorm;

    end->model.data.normal[at * 3] = x;
    end->model.data.normal[at * 3 + 1] = y;
    end->model.data.normal[at * 3 + 2] = z;

    end->model.element.atNorm++;
}

void processTriangleLine(char *line, uint32_t len)
{
    char *token, *next;
    char *indexTriplet[3];
    int32_t posInd, texInd, normInd;
    uint16_t at = end->model.element.atTriangle;
    line = &line[1];

    indexTriplet[0] = strtok_r(line, " ", &next);
    indexTriplet[1] = strtok_r(next, " ", &next);
    indexTriplet[2] = strtok_r(next, " ", &next);

    for(uint32_t i = 0; i < 3; i++)
    {
        token = strtok_r(indexTriplet[i], "/", &next);
        posInd = atoi(token);
        end->model.index.posIndex[at * 3 + i] = posInd - 1;
        if(end->model.element.atTex != 0)
        {
            token = strtok_r(next, "/", &next);
            texInd = atoi(token);
            end->model.index.texIndex[at * 3 + i] = texInd - 1;
        }
        token = strtok_r(next, "/", &next);
        normInd = atoi(token);
        end->model.index.normIndex[at * 3 + i] = normInd - 1;
    }
    end->model.element.atTriangle++;
}

void endModel()
{
    end->next = (RawModelList*)malloc(sizeof(RawModelList));
    end = end->next;
    end->next = NULL;

    modelCount++;
}

void makeVulkanBuffers(EngineBase *base, RenderScene *scene)
{
    uint32_t currIndex;
    float *currentVec;
    RawModelList *curr;
    uint32_t elementCount;
    uint32_t instanceDataFloatCount;

    float *bufferContent;
    float *instanceData;

    scene->memory.modelBuffers = (Model*)malloc(sizeof(Model) * (modelCount + 1));
    curr = &start;

    for(uint32_t i = 0; i < modelCount; i++)
    {
        bufferContent = (float*)malloc(sizeof(float) * curr->model.element.atTriangle * perTriangleFloatCount);
        instanceData = makeInstanceData(&instanceDataFloatCount, i);
        createDrawableModel(&curr->model, bufferContent);
        scene->memory.modelBuffers[i].vertexCount = curr->model.element.atTriangle * TRIANGLE_VERTEX_COUNT;
        scene->memory.modelBuffers[i].instanceCount = getButtonCount();
        makeVertexBuffer(base, scene, i, bufferContent, instanceData, scene->memory.modelBuffers[i].vertexCount * VERTEX_FLOAT_COUNT_STRIDE, instanceDataFloatCount);
        curr = curr->next;
        free(bufferContent);
        free(instanceData);
    }

    bufferContent = generateGameTableData(&scene->memory.modelBuffers[modelCount].vertexCount);
    makeVertexBuffer(base, scene, modelCount, bufferContent, ident, scene->memory.modelBuffers[modelCount].vertexCount * VERTEX_FLOAT_COUNT_STRIDE, sizeof(ident));
    scene->memory.modelBuffers[modelCount].instanceCount = 1;
}

static void createDrawableModel(RawModel *model, float *bufferContent)
{
    uint32_t currIndex;
    float *currentVec;

    for(uint32_t j = 0; j < model->element.atTriangle; j++)
    {
        for(uint32_t k = 0; k < 3; k++)
        {
            currIndex = model->index.posIndex[j * TRIANGLE_VERTEX_COUNT + k];
            currentVec = &model->data.position[currIndex * 3];
            memcpy(&bufferContent[j * perTriangleFloatCount + k * VERTEX_FLOAT_COUNT_STRIDE], currentVec, 3 * sizeof(float));

            if(model->element.atTex != 0)
            {
                currIndex = model->index.texIndex[j * TRIANGLE_VERTEX_COUNT + k];
                currentVec = &model->data.texture[currIndex * 2];
                memcpy(&bufferContent[j * perTriangleFloatCount + k * VERTEX_FLOAT_COUNT_STRIDE + 3], currentVec, 2 * sizeof(float));
            }

            currIndex = model->index.normIndex[j * TRIANGLE_VERTEX_COUNT + k];
            currentVec = &model->data.normal[currIndex * 3];
            memcpy(&bufferContent[j * perTriangleFloatCount + k * VERTEX_FLOAT_COUNT_STRIDE + 5], currentVec, 3 * sizeof(float));
        }
    }
}