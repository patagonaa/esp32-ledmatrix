#pragma once
#include<stdint.h>
#include "config.h"
#include "color.h"

extern union color frameBuffer[MAX_ANIMATION_FRAMES][NUM_PIXELS];
extern size_t frameCount;