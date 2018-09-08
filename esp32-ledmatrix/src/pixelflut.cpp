#include <WiFi.h>
#include <math.h>
#include "pixelflut.h"
#include "pixelbuf.h"

WiFiClient *clients[PIXELFLUT_MAX_CLIENTS] = {0};
char buffers[PIXELFLUT_MAX_CLIENTS][PIXELFLUT_MAX_MSG + 1] = {0};
uint16_t bufferPositions[PIXELFLUT_MAX_CLIENTS] = {0};

const char *handle_message(const char *msg);
void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

void handle_clients(WiFiServer *server)
{
    WiFiClient availableClient = server->available();
    WiFiClient *client = NULL;
    if (availableClient)
    {
        Serial.println("Client connected");
        for (int i = 0; i < PIXELFLUT_MAX_CLIENTS; i++)
        {
            if (clients[i] == NULL)
            {
                client = clients[i] = new WiFiClient(availableClient);
            }
        }
    }

    for (int i = 0; i < PIXELFLUT_MAX_CLIENTS; i++)
    {
        if (clients[i] == NULL)
            continue;
        client = clients[i];
        if (!client->connected())
        {
            Serial.println("remove disconnected client");
            client->stop();
            delete client;
            clients[i] = NULL;
            continue;
        }

        int available = client->available();
        uint16_t bufferPos = bufferPositions[i];

        for (int recvPos = 0; recvPos < available; recvPos++)
        {
            // if(recvPos > 64){
            //     Serial.printf("%d chars still in buffer, break to be fair\n", available - recvPos);
            //     break;
            // }

            char chr = client->read();
            buffers[i][bufferPos++] = chr;
            if (chr == '\n')
            {
                buffers[i][bufferPos] = '\0';
                const char *response = handle_message(buffers[i]);
                if (response != NULL)
                {
                    client->write(response);
                    client->flush();
                    delete response;
                }
                bufferPos = 0;
                continue;
                //break;
            }
            else if (bufferPos >= PIXELFLUT_MAX_MSG)
            {
                Serial.println("Message too large!");
                client->stop();
                break;
            }
        }

        bufferPositions[i] = bufferPos;
    }
}

const char *handle_message(const char *msg)
{
    Serial.println(msg);
    if (strncmp(msg, "PX", 2) == 0)
    {
        int x;
        int y;

        unsigned int hex0;
        unsigned int hex1;
        unsigned int hex2;
        unsigned int hex3;

        int paramCount = sscanf(msg + 2, "%d %d %2x%2x%2x%2x", &x, &y, &hex0, &hex1, &hex2, &hex3);

        if (paramCount == 3)
        {
            setPixel(x, y, hex0, hex0, hex0);
        }
        else if (paramCount == 5 || paramCount == 6)
        {
            setPixel(x, y, hex0, hex1, hex2);
        }
        else
        {
            Serial.print("Invalid param count ");
            Serial.println(paramCount);
            Serial.println(msg + 2);
        }
    }
    else if (strncmp(msg, "SIZE", 4) == 0)
    {
        char *response = new char[20];
        sprintf(response, "SIZE %d %d \n", (int)FRAME_WIDTH, (int)FRAME_HEIGHT);
        return response;
    }

    return NULL;
}

void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= FRAME_WIDTH || y >= FRAME_HEIGHT)
    {
        Serial.println("Invalid Position");
        return;
    }

    union color c = {0};

    c.parts.r1 = r;
    c.parts.r2 = r;
    c.parts.g = g;
    c.parts.b = b;

    frameBuffer[y * FRAME_WIDTH + x] = c;
}