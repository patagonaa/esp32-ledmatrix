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
        if (yHalf == 1)
            continue;

        for (uint16_t x = 0; x < FRAME_WIDTH; x++)
        {
            uint16_t pixelIndex = rowIndex + x;
            union color pixelTop = frameBuffer[pixelIndex];
            union color pixelBottom = frameBuffer[pixelIndex + (FRAME_WIDTH * (PANEL_HEIGHT / 2))];

            // pixelTop.parts.r1 = gamma8[pixelTop.parts.r1];
            // pixelTop.parts.r2 = gamma8[pixelTop.parts.r2];
            // pixelTop.parts.g =  gamma8[pixelTop.parts.g];
            // pixelTop.parts.b =  gamma8[pixelTop.parts.b];

            // pixelBottom.parts.r1 = gamma8[pixelBottom.parts.r1];
            // pixelBottom.parts.r2 = gamma8[pixelBottom.parts.r2];
            // pixelBottom.parts.g =  gamma8[pixelBottom.parts.g];
            // pixelBottom.parts.b =  gamma8[pixelBottom.parts.b];

            div_t panelXDiv = div(x, PANEL_WIDTH);
            int panelX = panelXDiv.quot;

            int panelIndex = panelMapping[panelX][panelY];
            size_t panelOffset = panelIndex * PANEL_OUTPUTBUFFER_LENGTH;

            div_t xQuarterDiv = div(panelXDiv.rem, PANEL_WIDTH / 4);
            int xQuarter = xQuarterDiv.quot;

            uint16_t row = yHalfDiv.rem;
            uint16_t col = xQuarterDiv.rem;

            size_t panelPixelIndex = xQuarter * 16 + row * 4 + col;
            for (size_t pwmCompare = 0; pwmCompare <= PWM_DEPTH; pwmCompare++)
            {
                size_t pwmCompareOffset = pwmCompare * PANELS_OUTPUTBUFFER_LENGTH;
                size_t outputBufferIndex = panelOffset + pwmCompareOffset + panelPixelIndex;

                uint8_t colorValue = getPixelColor2(pixelBottom, pwmCompare) | getPixelColor(pixelTop, pwmCompare);

                uint8_t bufferValue = (colorValue & 0xFE);

                if ((outputBufferIndex % PANELS_OUTPUTBUFFER_LENGTH) == PANELS_OUTPUTBUFFER_LENGTH - 1)
                {
                    bufferValue |= 0x01;
                }

                //causes flicker... wtf?
                //outputBuffer[outputBufferIndex] = bufferValue;

                outputBufferIndex ^= 0x02; //fix dma index

                outputBuffer[outputBufferIndex] = bufferValue;
                //outputBuffer[outputBufferIndex] &= ~(bufferValue);
                //outputBuffer[outputBufferIndex] |= bufferValue;
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