#pragma once
#include<stdint.h>
#include "config.h"

#define PANEL_OUTPUTBUFFER_LENGTH ((PANEL_WIDTH * PANEL_HEIGHT) / 2)

#define NUM_PIXELS (FRAME_WIDTH * FRAME_HEIGHT)

union color {
    uint32_t rrgb;
    struct
    {
        uint8_t r1;
        uint8_t r2;
        uint8_t g;
        uint8_t b;
    } parts;
};

extern union color frameBuffer[NUM_PIXELS];

void updateOutputBuffer(volatile uint8_t *outputBuffer);