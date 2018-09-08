#include <Arduino.h>
#include <stdlib.h>
#include "gamma8.h"
#include "color.h"
#include "config.h"

uint8_t getPixelColor(union color color, size_t pwmValue);
uint8_t getPixelColor2(union color color, size_t pwmValue);

void updateOutputBuffer(const union color *frameBuffer, volatile uint8_t *outputBuffer)
{
    for (uint16_t y = 0; y < FRAME_HEIGHT; y++)
    {
        uint16_t rowIndex = y * FRAME_WIDTH;
        div_t panelYDiv = div(y, PANEL_HEIGHT);
        int panelY = panelYDiv.quot;

        div_t yHalfDiv = div(panelYDiv.rem, PANEL_HEIGHT / 2);
        int yHalf = yHalfDiv.quot;

        for (uint16_t x = 0; x < FRAME_WIDTH; x++)
        {
            uint16_t pixelIndex = rowIndex + x;
            union color pixel = frameBuffer[pixelIndex];

            pixel.parts.r1 = gamma8[pixel.parts.r1] / 25;
            pixel.parts.r2 = gamma8[pixel.parts.r2] / 25;
            pixel.parts.g = gamma8[pixel.parts.g] / 25;
            pixel.parts.b = gamma8[pixel.parts.b] / 25;

            div_t panelXDiv = div(x, PANEL_WIDTH);
            int panelX = panelXDiv.quot;

            int panelIndex = panelMapping[panelX][panelY];

            div_t xQuarterDiv = div(panelXDiv.rem, 4);
            int xQuarter = xQuarterDiv.quot;

            uint16_t row = yHalfDiv.rem;
            uint16_t col = xQuarterDiv.rem;

            size_t panelPixelIndex = xQuarter * 16 + row * 4 + col;
            for (size_t pwmCompare = 0; pwmCompare <= PWM_DEPTH; pwmCompare++)
            {
                size_t panelOffset = panelIndex * PANEL_OUTPUTBUFFER_LENGTH;
                size_t pwmCompareOffset = pwmCompare * PANEL_OUTPUTBUFFER_LENGTH * PANELS;
                size_t outputBufferIndex = panelOffset + pwmCompareOffset + panelPixelIndex;

                //outputBuffer[outputBufferIndex] = ((outputBuffer[outputBufferIndex] & (half ? 0x0F : 0xF0))) | (getPixelColor(pixel, pwmCompare) << (half ? 4 : 0));
                outputBuffer[outputBufferIndex] = ((outputBuffer[outputBufferIndex] & (yHalf ? 0x0F : 0xF0))) | (yHalf ? getPixelColor2(pixel, pwmCompare) : getPixelColor(pixel, pwmCompare));
            }
        }
    }
}

uint8_t getPixelColor(union color color, size_t pwmValue)
{
    uint8_t toReturn = 0;
    if (color.parts.r1 > pwmValue)
    {
        toReturn |= 0x01;
    }
    if (color.parts.r2 > pwmValue)
    {
        toReturn |= 0x02;
    }
    if (color.parts.g > pwmValue)
    {
        toReturn |= 0x04;
    }
    if (color.parts.b > pwmValue)
    {
        toReturn |= 0x08;
    }

    return toReturn;
}

uint8_t getPixelColor2(union color color, size_t pwmValue)
{
    uint8_t toReturn = 0;
    if (color.parts.r1 > pwmValue)
    {
        toReturn |= 0x10;
    }
    if (color.parts.r2 > pwmValue)
    {
        toReturn |= 0x20;
    }
    if (color.parts.g > pwmValue)
    {
        toReturn |= 0x40;
    }
    if (color.parts.b > pwmValue)
    {
        toReturn |= 0x80;
    }

    return toReturn;
}