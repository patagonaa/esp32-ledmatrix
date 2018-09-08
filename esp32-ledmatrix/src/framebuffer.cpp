#include <stddef.h>
#include "framebuffer.h"
#include "color.h"

union color frameBuffer[MAX_ANIMATION_FRAMES][NUM_PIXELS] = {0};
size_t frameCount = 0;