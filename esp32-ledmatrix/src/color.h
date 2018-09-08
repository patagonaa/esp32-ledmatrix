#pragma once
#include<stdint.h>

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