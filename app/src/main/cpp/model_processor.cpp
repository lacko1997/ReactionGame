//
// Created by laszlo on 4/28/26.
//

#include "model_processor.h"

#include <string.h>
#include <stdlib.h>

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
    AtElement elment;
    RawModelData data;
    RawModelIndices index;
}RawModel;

RawModel currentModel;

void beginModel(uint16_t vertexCount, uint16_t uvCount, uint16_t normalCount, uint16_t triangleCount)
{
    currentModel.data.position = (float*)malloc(sizeof(float) * 3 * vertexCount);
    currentModel.data.texture = (float*)malloc(sizeof(float) * 2 * uvCount);
    currentModel.data.normal = (float*)malloc(sizeof(float) * 3 * normalCount);

    currentModel.index.posIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);
    currentModel.index.texIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);
    currentModel.index.normIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);
}

void processPositionLine(char *line, uint32_t len)
{
    char *token;
    float x, y, z;
    if(line != NULL && len != 0)
    {
        line = &line[1];
        token = strtok(line, " ");
        x = atof(token);
        token = strtok(line, " ");
        y = atof(token);
        token = strtok(line, " ");
        z = atof(token);
    }
    currentModel.
}

void processTextureLine(char *line, uint32_t len)
{

}

void processNormalLine(char *line, uint32_t len)
{

}

void processTriangleLine(char *line, uint32_t len)
{

}