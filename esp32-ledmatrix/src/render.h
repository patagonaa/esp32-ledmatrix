#pragma once
#include<stdint.h>
#include "config.h"
#include "color.h"

void updateOutputBuffer(const union color * frameBuffer, volatile uint8_t *outputBuffer);