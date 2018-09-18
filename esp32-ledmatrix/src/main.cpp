#include <Arduino.h>
#include <stdlib.h>
#include <WiFi.h>
#include <ArtnetWifi.h>
#include "config.h"
#include "render.h"
#include "framebuffer.h"
#include "gamma8.h"

void sendFrame();
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data);

uint8_t outputBuffer[PANELS * PANEL_OUTPUTBUFFER_LENGTH * (PWM_DEPTH + 1)] = {0};
volatile size_t outputPwmCompare = 0;
unsigned long nextFrameAt = 0;

ArtnetWifi artnet;

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

    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
}

const uint32_t clkPinMask = (uint32_t)1 << clkPin;
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
    Serial.begin(921000);
    setupPins();

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        frameBuffer[i].parts.r1 = 1;
        frameBuffer[i].parts.r2 = 1;
        frameBuffer[i].parts.g = 1;
        frameBuffer[i].parts.b = 1;
    }

    updateOutputBuffer(frameBuffer, outputBuffer);

    setupWifi();
    setupLookupTables();
    setupTimers();
}

void loop()
{
    unsigned long ms = millis();

    if (ms > nextFrameAt && outputBufferDirty)
    {
        //Serial.print(";");
        outputBufferDirty = false;
        //unsigned long start = micros();
        updateOutputBuffer(frameBuffer, outputBuffer);
        //unsigned long end = micros();
        //Serial.print("Render:");
        //Serial.println(end - start);
        nextFrameAt = ms + 40;
    }

    artnet.read();
}

void sendFrame()
{
    if (outputPwmCompare > PWM_DEPTH)
    {
        outputPwmCompare = 0;
    }

    size_t pwmBufferShift = PANELS_OUTPUTBUFFER_LENGTH * outputPwmCompare;

    digitalWrite(latchPin, false);
    //GPIO.out_w1tc = (uint32_t)1 << latchPin;
    size_t bufferPtr = pwmBufferShift;
    for (size_t i = 0; i < PANELS_OUTPUTBUFFER_LENGTH; i++)
    {
        uint8_t bufferValue = outputBuffer[bufferPtr++];
        GPIO.out_w1tc = clkPinMask;

        uint32_t colorPinsValue = colorPinsLookup[bufferValue];
        GPIO.out_w1ts = colorPinsValue & colorPinsMask;
        GPIO.out_w1tc = (~colorPinsValue) & colorPinsMask;

        GPIO.out_w1ts = clkPinMask;
    }
    digitalWrite(latchPin, true);
    //GPIO.out_w1ts = (uint32_t)1 << latchPin;

    outputPwmCompare++;
}