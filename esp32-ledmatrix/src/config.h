#pragma once

#define NUM_COLOR_PINS 8

#define PANELS_X 1
#define PANELS_Y 2

#define PANEL_WIDTH 16
#define PANEL_HEIGHT 8

#define PWM_DEPTH (1 << 8)

#define PIXELFLUT_PORT 1337

#define MAX_ANIMATION_FRAMES 1

#define PANELS (PANELS_X * PANELS_Y)

#define FRAME_WIDTH (PANEL_WIDTH * PANELS_X)
#define FRAME_HEIGHT (PANEL_HEIGHT * PANELS_Y)

#define PANEL_OUTPUTBUFFER_LENGTH ((PANEL_WIDTH * PANEL_HEIGHT) / 2)

#define NUM_PIXELS (FRAME_WIDTH * FRAME_HEIGHT)

#define OUTPUTBUFFER_BYTES (PANELS * PANEL_OUTPUTBUFFER_LENGTH * (PWM_DEPTH + 1))

extern const int colorPins[NUM_COLOR_PINS];

extern const int clkPin;
extern const int latchPin;
extern const int oePin;

extern const char *ssid;
extern const char *password;

extern const int panelMapping[PANELS_X][PANELS_Y];