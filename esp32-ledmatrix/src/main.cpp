#include <Arduino.h>
#include <stdlib.h>
#include <WiFi.h>
#include <ArtnetWifi.h>
#include "config.h"
#include "render.h"
#include "framebuffer.h"
#include "gamma8.h"
#include "4x6_horizontal_MSB_1.h"

extern "C"
{
#include "dma/i2s_parallel.h"
}

void sendFrame();
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data);

uint8_t outputBuffer[OUTPUTBUFFER_BYTES] = {0};

volatile size_t outputPwmCompare = 0;
unsigned long nextFrameAt = 0;
bool startScreen = true;

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

    ledcSetup(0, 100000, 8);
    ledcAttachPin(oePin, 0);
    ledcWrite(0, 220); //TODO: set via artnet
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
        frameBuffer[i].parts.r1 = 0;
        frameBuffer[i].parts.r2 = 0;
        frameBuffer[i].parts.g = 0;
        frameBuffer[i].parts.b = 0;
    }

    updateOutputBuffer(frameBuffer, outputBuffer);

    setupWifi();
    setupI2s();
}

void drawChar(uint8_t chr, int x, int y)
{
    for (int charY = 0; charY < 6; charY++)
    {
        for (int charX = 0; charX < 4; charX++)
        {
            bool px = (font[chr][charY] & (1 << (3 - charX)));
            frameBuffer[((y + charY) * FRAME_WIDTH) + x + charX].rrgb = px ? 0x01010101 : 0x00000000;
        }
    }
}

void drawIpPart(uint32_t part)
{
    for (size_t i = 0; i < NUM_PIXELS; i++)
    {
        frameBuffer[i].rrgb = 0;
    }

    IPAddress ip = WiFi.localIP();
    int ipPartNum = part % 6;
    if (ipPartNum < 4)
    {
        char ipPartStr[4];
        sprintf(ipPartStr, "%d", ip[ipPartNum]);
        for (int i = 0; i < 3; i++)
        {
            if (ipPartStr[i] == '\0')
                break;
            drawChar(ipPartStr[i], i * 4, 0);
        }
    }
}

void loop()
{
    if (startScreen)
    {
        unsigned long ms = millis();
        if (artnet.read())
            startScreen = false;
        drawIpPart(ms / 1000);
        updateOutputBuffer(frameBuffer, outputBuffer);
    }
    else
    {
        if (!artnet.read())
            return;

        unsigned long ms;
        if (outputBufferDirty && (ms = millis()) > nextFrameAt)
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
    }
}