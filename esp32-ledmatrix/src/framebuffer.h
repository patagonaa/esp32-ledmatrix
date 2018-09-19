#pragma once
#include<stdint.h>
#include "config.h"
#include "color.h"

extern union color frameBuffer[NUM_PIXELS];
extern volatile bool outputBufferDirty;
