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

typedef struct RawModelList
{
    RawModel model;
    RawModelList *next;
}RawModelList;

RawModelList start;
RawModelList *end = &start;

void beginModel(uint16_t vertexCount, uint16_t uvCount, uint16_t normalCount, uint16_t triangleCount)
{
    end->model.data.position = (float*)malloc(sizeof(float) * 3 * vertexCount);
    end->model.data.texture = (float*)malloc(sizeof(float) * 2 * uvCount);
    end->model.data.normal = (float*)malloc(sizeof(float) * 3 * normalCount);

    end->model.index.posIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);
    end->model.index.texIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);
    end->model.index.normIndex = (uint16_t*)malloc(sizeof(uint16_t) * triangleCount);

    end->model.elment.atPos = 0;
    end->model.elment.atTex = 0;
    end->model.elment.atNorm = 0;
    end->model.elment.atTriangle = 0;
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
    at = end->model.elment.atPos;

    end->model.data.position[at * 3] = x;
    end->model.data.position[at * 3 + 1] = y;
    end->model.data.position[at * 3 + 2] = z;

    end->model.elment.atPos++;
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
    at = end->model.elment.atTex;

    end->model.data.texture[at * 2] = x;
    end->model.data.texture[at * 2 + 1] = y;

    end->model.elment.atTex++;
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
    at = end->model.elment.atNorm;

    end->model.data.normal[at * 3] = x;
    end->model.data.normal[at * 3 + 1] = y;
    end->model.data.normal[at * 3 + 2] = z;

    end->model.elment.atNorm++;
}

void processTriangleLine(char *line, uint32_t len)
{
    char *token, *next;
    char *indexTriplet[3];
    int32_t posInd, texInd, normInd;
    uint16_t at = end->model.elment.atTriangle;
    line = &line[1];

    indexTriplet[0] = strtok_r(line, " ", &next);
    indexTriplet[1] = strtok_r(next, " ", &next);
    indexTriplet[2] = strtok_r(next, " ", &next);

    for(uint32_t i = 0; i < 3; i++)
    {
        token = strtok_r(indexTriplet[i], "/", &next);
        posInd = atoi(token);
        end->model.index.posIndex[at + i] = posInd - 1;
        if(end->model.elment.atTex != 0)
        {
            token = strtok_r(next, "/", &next);
            texInd = atoi(token);
            end->model.index.texIndex[at + i] = texInd - 1;
        }
        strtok_r(next, "/", &next);
        normInd = atoi(token);
        end->model.index.normIndex[at + i] = normInd -1;
    }
    end->model.elment.atTriangle++;
}

