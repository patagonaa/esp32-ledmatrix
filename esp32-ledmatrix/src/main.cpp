#include <Arduino.h>
#include <stdlib.h>
#include <WiFi.h>
#include <ArtnetWifi.h>
#include "config.h"
#include "render.h"
#include "framebuffer.h"
#include "gamma8.h"

extern "C"
{
#include "dma/i2s_parallel.h"
}

void sendFrame();
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data);

uint8_t outputBuffer[OUTPUTBUFFER_BYTES] = {0};

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
    //timerAlarmEnable(timer);
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

void setupI2s()
{
    i2s_parallel_buffer_desc_t bufdesc;
    i2s_parallel_config_t cfg = {
        .gpio_bus = {latchPin, colorPins[1], colorPins[2], colorPins[3], colorPins[4], colorPins[5], colorPins[6], colorPins[7]},
        .gpio_clk = clkPin,
        .clkspeed_hz = 10 * 1000 * 1000,
        .bits = I2S_PARALLEL_BITS_8,
        .buf = &bufdesc,
        //.bufb = bufdesc[1],
    };

    bufdesc.memory = outputBuffer;
    bufdesc.size = OUTPUTBUFFER_BYTES;

    i2s_parallel_setup(&I2S1, &cfg);
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
    setupI2s();
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