#include <Arduino.h>
#include <stdlib.h>
#include <WiFi.h>
#include "config.h"
#include "pixelflut.h"
#include "render.h"
#include "framebuffer.h"
#include "gamma8.h"

void sendFrame();

size_t currentDisplayFrame = 0;

volatile uint8_t outputBuffer[PANELS * PANEL_OUTPUTBUFFER_LENGTH * (PWM_DEPTH + 1)] = {0};
volatile size_t outputPwmCompare = 0;

WiFiServer server(PIXELFLUT_PORT, PIXELFLUT_MAX_CLIENTS);

void setupPins()
{
    for (int i = 0; i < NUM_COLOR_PINS; i++)
    {
        pinMode(colorPins[i], OUTPUT);
    }
    pinMode(clkPin, OUTPUT);
    pinMode(latchPin, OUTPUT);

    pinMode(oePin, OUTPUT);
    digitalWrite(oePin, false);
}

void setupTimers()
{
    hw_timer_t *timer = NULL;
    timer = timerBegin(0, 80, true);

    timerAttachInterrupt(timer, &sendFrame, true);
    timerAlarmWrite(timer, 50, true);
    timerAlarmEnable(timer);
}

void setupWifi()
{
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();
}

uint32_t colorPinsMask = 0;
uint32_t colorPinsLookup[256] = {0};

void setupLookupTables()
{
    for (uint16_t i = 0; i < 256; i++)
    {
        uint8_t bufferValue = i;
        for (int j = 0; j < NUM_COLOR_PINS; j++)
        {
            uint32_t pinMask = (uint32_t)1 << colorPins[j];
            colorPinsLookup[i] |= (bufferValue & 0x01) ? pinMask : 0;
            colorPinsMask |= pinMask;
            bufferValue >>= 1;
        }
    }
}

void setup()
{
    Serial.begin(115200);
    setupPins();

    frameCount = 1;
    currentDisplayFrame = 0;

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        frameBuffer[currentDisplayFrame][i].parts.r1 = 0;
        frameBuffer[currentDisplayFrame][i].parts.r2 = 0;
        frameBuffer[currentDisplayFrame][i].parts.g = 0;
        frameBuffer[currentDisplayFrame][i].parts.b = 0;
    }

    updateOutputBuffer(frameBuffer[currentDisplayFrame], outputBuffer);

    setupWifi();
    setupLookupTables();
    setupTimers();
}

void loop()
{
    unsigned long ms = millis();
    unsigned static long lastUpdateMillis = 0;

    if (ms > lastUpdateMillis + 100)
    {

        if(currentDisplayFrame >= frameCount){
            currentDisplayFrame = 0;
        }
        unsigned long usStart = micros();
        updateOutputBuffer(frameBuffer[currentDisplayFrame], outputBuffer);
        unsigned long usEnd = micros();
        Serial.println(usEnd - usStart);
        lastUpdateMillis = ms;
        currentDisplayFrame++;
    }

    handle_clients(&server);

    // for (int i = 0; i < NUM_PIXELS; i++)
    // {
    //     frameBuffer[i].parts.r1 = 255;
    //     frameBuffer[i].parts.r2 = 255;
    //     frameBuffer[i].parts.g = 255;
    //     frameBuffer[i].parts.b = 255;
    // }
    // updateOutputBuffer(outputBuffer);
}

void sendFrame()
{
    if (outputPwmCompare > PWM_DEPTH)
    {
        outputPwmCompare = 0;
    }

    size_t pwmBufferShift = PANEL_OUTPUTBUFFER_LENGTH * PANELS * outputPwmCompare;

    digitalWrite(latchPin, false);
    //GPIO.out_w1tc = (uint32_t)1 << latchPin;

    for (size_t i = 0; i < PANEL_OUTPUTBUFFER_LENGTH * PANELS; i++)
    {
        uint8_t bufferValue = outputBuffer[i + pwmBufferShift];
        GPIO.out_w1tc = (uint32_t)1 << clkPin;

        GPIO.out_w1ts = colorPinsLookup[bufferValue] & colorPinsMask;
        GPIO.out_w1tc = (~colorPinsLookup[bufferValue]) & colorPinsMask;

        GPIO.out_w1ts = (uint32_t)1 << clkPin;
    }
    digitalWrite(latchPin, true);
    //GPIO.out_w1ts = (uint32_t)1 << latchPin;

    outputPwmCompare++;
}
