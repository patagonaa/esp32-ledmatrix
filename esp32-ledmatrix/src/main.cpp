#include <Arduino.h>
#include <stdlib.h>
#include <WiFi.h>
#include "config.h"
#include "pixelflut.h"
#include "render.h"
#include "framebuffer.h"
#include "gamma8.h"

extern "C"
{
#include "dma/i2s_parallel.h"
}

void sendFrame();

size_t currentDisplayFrame = 0;

volatile uint8_t outputBuffer[OUTPUTBUFFER_BYTES] = {0};
//uint8_t outputBuffer2[OUTPUTBUFFER_BYTES] = {0};
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

    server.begin();
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
    Serial.begin(115200);
    setupPins();

    frameCount = 1;
    currentDisplayFrame = 0;

    for (int i = 0; i < NUM_PIXELS; i++)
    {
        frameBuffer[currentDisplayFrame][i].parts.r1 = 32;
        frameBuffer[currentDisplayFrame][i].parts.r2 = 32;
        frameBuffer[currentDisplayFrame][i].parts.g = 32;
        frameBuffer[currentDisplayFrame][i].parts.b = 32;
    }

    updateOutputBuffer(frameBuffer[currentDisplayFrame], outputBuffer);

    setupWifi();
    setupI2s();
}

void loop()
{
    unsigned long ms = millis();
    unsigned static long lastUpdateMillis = 0;

    if (ms > lastUpdateMillis + 100)
    {
        if (currentDisplayFrame >= frameCount)
        {
            currentDisplayFrame = 0;
        }
        //unsigned long usStart = micros();
        updateOutputBuffer(frameBuffer[currentDisplayFrame], outputBuffer);
        //unsigned long usEnd = micros();
        //Serial.println(usEnd - usStart);
        lastUpdateMillis = ms;
        currentDisplayFrame++;
    }

    //i2s_parallel_flip_to_buffer(&I2S1, 0);

    handle_clients(&server);
}
