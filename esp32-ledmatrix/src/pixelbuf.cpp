#include <Arduino.h>
#include <stdlib.h>
#include "gamma8.h"
#include "pixelbuf.h"
#include "config.h"

uint8_t getPixelColor(union color color, int pwmValue);

union color frameBuffer[NUM_PIXELS] = {0};

void updateOutputBuffer(volatile uint8_t *outputBuffer)
{
    for (uint16_t y = 0; y < FRAME_HEIGHT; y++)
    {
        uint16_t rowIndex = y * FRAME_WIDTH;
        for (uint16_t x = 0; x < FRAME_WIDTH; x++)
        {
            uint16_t pixelIndex = rowIndex + x;
            union color pixel = frameBuffer[pixelIndex];

            pixel.parts.r1 = gamma8[pixel.parts.r1];
            pixel.parts.r2 = gamma8[pixel.parts.r2];
            pixel.parts.g = gamma8[pixel.parts.g];
            pixel.parts.b = gamma8[pixel.parts.b];

            div_t panelXDiv = div(x, PANEL_WIDTH);
            int panelX = panelXDiv.quot;

            div_t panelYDiv = div(y, PANEL_HEIGHT);
            int panelY = panelYDiv.quot;
            int panelIndex = panelMapping[panelX][panelY];

            div_t halfDiv = div(panelYDiv.rem, 4);
            int half = halfDiv.quot;

            div_t quarterDiv = div(panelXDiv.rem, 4);
            int quarter = quarterDiv.quot;

            int row = halfDiv.rem;
            int col = quarterDiv.rem;

            size_t panelPixelIndex = quarter * 16 + row * 4 + col;
            for (size_t pwmCompare = 0; pwmCompare < PWM_DEPTH; pwmCompare++)
            {
                size_t panelOffset = panelIndex * PANEL_OUTPUTBUFFER_LENGTH;
                size_t pwmCompareOffset = pwmCompare * PANEL_OUTPUTBUFFER_LENGTH * PANELS;
                size_t outputBufferIndex = panelOffset + pwmCompareOffset + panelPixelIndex;
                // if(getPixelColor(pixel, pwmCompare) != 0){
                //     Serial.println("-----");
                //     Serial.println(panelOffset);
                //     Serial.println("-----");
                // }
                outputBuffer[outputBufferIndex] = ((outputBuffer[outputBufferIndex] & (half ? 0x0F : 0xF0))) | (getPixelColor(pixel, pwmCompare) << (half ? 4 : 0));
            }
        }
    }

    // Serial.println("-----");

    // for (size_t i = 0; i < PANEL_OUTPUTBUFFER_LENGTH * PANELS * PWM_DEPTH; i++)
    // {
    //     if (i % PANEL_WIDTH == 0)
    //     {
    //         Serial.println();
    //     }
    //     if (i % PANEL_OUTPUTBUFFER_LENGTH == 0)
    //     {
    //         Serial.println();
    //     }
    //     Serial.print(outputBuffer[i], 16);
    //     Serial.print(" ");
    // }

    // Serial.println("-----");
}

uint8_t getPixelColor(union color color, int pwmValue)
{
    uint8_t toReturn = 0;
    if (color.parts.r1 > pwmValue || color.parts.r1 == PWM_DEPTH - 1)
    {
        toReturn |= 1;
    }
    if (color.parts.r2 > pwmValue || color.parts.r2 == PWM_DEPTH - 1)
    {
        toReturn |= 1 << 1;
    }
    if (color.parts.g > pwmValue || color.parts.g == PWM_DEPTH - 1)
    {
        toReturn |= 1 << 2;
    }
    if (color.parts.b > pwmValue || color.parts.b == PWM_DEPTH - 1)
    {
        toReturn |= 1 << 3;
    }

    return toReturn;
}