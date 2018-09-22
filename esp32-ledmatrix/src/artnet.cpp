#include <Arduino.h>
#include <stddef.h>
#include "artnet.h"
#include "config.h"
#include "gamma8.h"
#include "framebuffer.h"

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
    uint16_t panelNum = universe - ARTNET_START_UNIVERSE;

    //one Artnet universe per display
    if (panelNum < 0 || panelNum >= PANELS)
    {
        return;
    }

    Serial.print(".");
    //Serial.println(sequence);

    size_t universePixels = PANEL_WIDTH * PANEL_HEIGHT;

    size_t pixelLength = length / 3;
    size_t endIndex = pixelLength > universePixels ? universePixels : pixelLength;
    for (size_t i = 0; i < endIndex; i++)
    {
        size_t pixelIndex = i * 3;
        uint8_t r = gamma8[data[pixelIndex]]     >> (8 - PWM_BITS);
        uint8_t g = gamma8[data[pixelIndex + 1]] >> (8 - PWM_BITS);
        uint8_t b = gamma8[data[pixelIndex + 2]] >> (8 - PWM_BITS);

        uint32_t rrgb = b << 24 | g << 16 | r << 8 | r;

        frameBuffer[panelNum * universePixels + i].rrgb = rrgb;
    }
    outputBufferDirty = true;
}