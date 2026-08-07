//
// Created by laszlo on 6/27/26.
//

#include "game_params.h"

#include <stdlib.h>
#include <string.h>

#define BUTTONS_PER_LINE (3)
#define BUTTON_MODEL_COUNT (3)

#define QUAD_FLOAT_COUNT (48)

static const float size = 12.0f;

static uint8_t bulbCount = 9;
static uint8_t arrangement = E_ARRANGEMENT_RECTANGULAR;

static void createRectangularBaseInstanceData(float *data);
static void createRectangularBulbInstanceData(float *data);
static void createRectangularButtonInstanceData(float *data);
static float *generateGameTableRectArrangement(uint32_t *vertexCount);

typedef void (*PFN_instanceDataCreator)(float*);
PFN_instanceDataCreator createRectangular[BUTTON_MODEL_COUNT] = {
        createRectangularBulbInstanceData,
        createRectangularBaseInstanceData,
        createRectangularButtonInstanceData
};

float *makeInstanceData(uint32_t *instanceDataFloatCount, uint32_t modelIndex)
{
    float *data = (float*) malloc(sizeof(float) * bulbCount * 16);
    *instanceDataFloatCount = bulbCount * 16;
    if(arrangement == E_ARRANGEMENT_RECTANGULAR)
    {
        createRectangular[modelIndex](data);
    }
    return data;
}

float *generateGameTableData(uint32_t *vertexCount)
{
    float *data = NULL;
    if(arrangement == E_ARRANGEMENT_RECTANGULAR)
    {
        data = generateGameTableRectArrangement(vertexCount);
    }
    else if(arrangement == E_ARRANGEMENT_CIRCULAR)
    {

    }
    return data;
}

uint32_t getButtonCount()
{
    return bulbCount;
}

void setButtonCount(uint32_t count)
{
    if(3 <= bulbCount && bulbCount <= 9)
    {
        bulbCount = count;
    }
}

static float *generateGameTableRectArrangement(uint32_t *vertexCount)
{

    uint8_t wholeLineCount = bulbCount / BUTTONS_PER_LINE;
    uint8_t countInLastLine = bulbCount % BUTTONS_PER_LINE;
    uint8_t partialLine = (countInLastLine != 0);
    float lineCount = (float)(wholeLineCount + partialLine);

    uint32_t quadCount = wholeLineCount * (BUTTONS_PER_LINE + 2) + (countInLastLine + 2) + 1;
    float *data = (float*)malloc(sizeof(float) * quadCount * QUAD_FLOAT_COUNT);

    float hSpacing = (2.0f * size) / (lineCount + 1);
    float wSpacing = (2.0f * size) / (BUTTONS_PER_LINE + 1);
    float lastWSpacing = (2.0f * size) / ((float)(countInLastLine + 1));

    *vertexCount = quadCount * 6;

    float topQuad[] =
            {
                    -size, 0.0f, -size + hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                    size, 0.0f, -size, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                    -size, 0.0f, -size, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                    -size, 0.0f, -size +  hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                    size, 0.0f, -size + hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                    size, 0.0f, -size, 1.0, 1.0f, 0.0f, 1.0f, 0.0f
            };
    memcpy(&data[0], topQuad, QUAD_FLOAT_COUNT * sizeof(float));
    for(uint32_t y = 1; y < (uint32_t)lineCount; y++)
    {
        float upperQuad[QUAD_FLOAT_COUNT] =
                {
                        -size, 0.0f, -size + ((float)y + 1.0f) * hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                        size, 0.0f, -size + ((float)y) * hSpacing + 1.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                        -size, 0.0f, -size + ((float)y) * hSpacing + 1.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                        -size, 0.0f, -size + ((float)y + 1.0f) * hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                        size, 0.0f, -size + ((float)y + 1.0f) * hSpacing - 3.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f,
                        size, 0.0f, -size + ((float)y) * hSpacing + 1.0f, 1.0, 1.0f, 0.0f, 1.0f, 0.0f
                };
        memcpy(&data[QUAD_FLOAT_COUNT * y * (BUTTONS_PER_LINE + 2)], upperQuad, QUAD_FLOAT_COUNT * sizeof(float));
    }

    for(uint32_t y = 0; y < wholeLineCount; y++)
    {
        float leftQuad[QUAD_FLOAT_COUNT] =
                {
                        -size, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size,  0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                };
        memcpy(&data[QUAD_FLOAT_COUNT * (y * (BUTTONS_PER_LINE + 2) + 1)], leftQuad, QUAD_FLOAT_COUNT * sizeof(float));
        for(uint32_t x = 1; x < BUTTONS_PER_LINE; x++)
        {
            float betweenQuad[QUAD_FLOAT_COUNT] =
                    {
                        -size + x * wSpacing + 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + (x + 1.0f) * wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + x * wSpacing + 1.0f,  0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + x * wSpacing + 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + (x + 1.0f) * wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + (x + 1.0f) * wSpacing - 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                    };
            memcpy(&data[QUAD_FLOAT_COUNT * (y * (BUTTONS_PER_LINE + 2) + x + 1)], betweenQuad, QUAD_FLOAT_COUNT * sizeof(float));
        }
        float rightQuad[QUAD_FLOAT_COUNT] =
                {
                        size - wSpacing + 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size - wSpacing + 1.0f,  0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size - wSpacing + 1.0f, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, -size + (y + 1.0f) * hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, -size + (y + 1.0f) * hSpacing - 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                };
        memcpy(&data[QUAD_FLOAT_COUNT * (y * (BUTTONS_PER_LINE + 2) + BUTTONS_PER_LINE + 1)], rightQuad, QUAD_FLOAT_COUNT * sizeof(float));
    }
    if(partialLine != 0)
    {
        float partialLeft[QUAD_FLOAT_COUNT] =
                {
                        -size, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size,  0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        -size + lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                };
        memcpy(&data[QUAD_FLOAT_COUNT * (BUTTONS_PER_LINE * (wholeLineCount + 2))], partialLeft, QUAD_FLOAT_COUNT * sizeof(float));
        for(uint32_t x = 1; x < partialLine; x++)
        {
            float partialBetween[QUAD_FLOAT_COUNT] =
                    {
                            -size + x * wSpacing + 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                            -size + (x + 1.0f) * lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                            -size + x * wSpacing + 1.0f,  0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                            -size + x * wSpacing + 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                            -size + (x + 1.0f) * lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                            -size + (x + 1.0f) * lastWSpacing - 1.0f, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                    };
            memcpy(&data[QUAD_FLOAT_COUNT * (BUTTONS_PER_LINE * (wholeLineCount + 2) + 1 + x)], partialBetween, QUAD_FLOAT_COUNT * sizeof(float));
        }
        float partialRight[QUAD_FLOAT_COUNT] =
                {
                        size - wSpacing + 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size - wSpacing + 1.0f,  0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size - wSpacing + 1.0f, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, size - lineCount * hSpacing - 1.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f,
                        size, 0.0f, size - lineCount * hSpacing + 3.0f, 0.0f, 0.0f, 0.0f, 1.0f , 0.0f
                };
        memcpy(&data[QUAD_FLOAT_COUNT * (BUTTONS_PER_LINE * (wholeLineCount + 2) + 1 + partialLine)], partialRight, QUAD_FLOAT_COUNT * sizeof(float));
    }

    float bottomQuad[QUAD_FLOAT_COUNT] =
            {
                -size, 0.0f, size, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                size, 0.0f, size - hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                -size, 0.0f, size - hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                -size, 0.0f, size, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                size, 0.0f, size, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                size, 0.0f, size - hSpacing + 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            };
    memcpy(&data[QUAD_FLOAT_COUNT * (quadCount - 1)], bottomQuad, QUAD_FLOAT_COUNT * sizeof(float));

    return data;
}

static void createRectangularBaseInstanceData(float *data)
{
    float xBulbDistance;
    float yBulbDistance;

    uint8_t wholeLineCount = bulbCount / BUTTONS_PER_LINE;
    uint8_t countInLastLine = bulbCount % BUTTONS_PER_LINE;
    uint8_t partialLine = (countInLastLine != 0);
    float lineCount = (float)(wholeLineCount + partialLine);

    memset(data, 0, bulbCount * 16 * sizeof(float));

    for(uint8_t y = 0; y < wholeLineCount; y++)
    {
        yBulbDistance = size * (2.0f / (lineCount + 1.0f));
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (BUTTONS_PER_LINE + 1.0f));

            data[(y * BUTTONS_PER_LINE + x) * 16 + 2] = -1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 8] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 12] = -size + (float)(x + 1) * xBulbDistance;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 14] = -size + (float)(y + 1) * yBulbDistance;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
    if(partialLine)
    {
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (((float)countInLastLine) + 1.0f));

            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 10] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 12] = -size + ((float)(x + 1)) * xBulbDistance;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 14] = -size + ((float)lineCount) * yBulbDistance;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
}

static void createRectangularBulbInstanceData(float *data)
{
    float xBulbDistance;
    float yBulbDistance;

    uint8_t wholeLineCount = bulbCount / BUTTONS_PER_LINE;
    uint8_t countInLastLine = bulbCount % BUTTONS_PER_LINE;
    uint8_t partialLine = (countInLastLine != 0);
    float lineCount = (float)(wholeLineCount + partialLine);

    memset(data, 0, bulbCount * 16 * sizeof(float));

    for(uint8_t y = 0; y < wholeLineCount; y++)
    {
        yBulbDistance = size * (2.0f / (lineCount + 1.0f));
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (BUTTONS_PER_LINE + 1.0f));

            data[(y * BUTTONS_PER_LINE + x) * 16] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 10] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 12] = -size + (float)(x + 1) * xBulbDistance;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 14] = -size + (float)(y + 1) * yBulbDistance - 2.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
    if(partialLine)
    {
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (((float)countInLastLine) + 1.0f));

            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 10] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 12] = -size + ((float)(x + 1)) * xBulbDistance;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 14] = -size + ((float)lineCount) * yBulbDistance + 4.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
}

static void createRectangularButtonInstanceData(float *data)
{
    float xBulbDistance;
    float yBulbDistance;

    uint8_t wholeLineCount = bulbCount / BUTTONS_PER_LINE;
    uint8_t countInLastLine = bulbCount % BUTTONS_PER_LINE;
    uint8_t partialLine = (countInLastLine != 0);
    float lineCount = (float)(wholeLineCount + partialLine);

    memset(data, 0, bulbCount * 16 * sizeof(float));

    for(uint8_t y = 0; y < wholeLineCount; y++)
    {
        yBulbDistance = size * (2.0f / (lineCount + 1.0f));
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (BUTTONS_PER_LINE + 1.0f));

            data[(y * BUTTONS_PER_LINE + x) * 16] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 10] = 1.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 12] = -size + (float)(x + 1) * xBulbDistance;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 14] = -size + (float)(y + 1) * yBulbDistance;
            data[(y * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
    if(partialLine)
    {
        for (uint8_t x = 0; x < BUTTONS_PER_LINE; x++)
        {
            xBulbDistance = size * (2.0f / (((float)countInLastLine) + 1.0f));

            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 5] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 10] = 1.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 12] = -size + ((float)(x + 1)) * xBulbDistance;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 13] = 0.0f;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 14] = -size + ((float)lineCount) * yBulbDistance;
            data[(wholeLineCount * BUTTONS_PER_LINE + x) * 16 + 15] = 1.0f;
        }
    }
}

