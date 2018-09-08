#pragma once

#define NUM_COLOR_PINS 8

#define PANELS_X 1
#define PANELS_Y 2

#define PANELS (PANELS_X * PANELS_Y)

extern const int panelMapping[PANELS_X][PANELS_Y];

#define PANEL_WIDTH 16
#define PANEL_HEIGHT 8

#define FRAME_WIDTH (PANEL_WIDTH * PANELS_X)
#define FRAME_HEIGHT (PANEL_HEIGHT * PANELS_Y)

#define PWM_DEPTH 255

#define PIXELFLUT_PORT 1337

extern const int colorPins[NUM_COLOR_PINS];

extern const int clkPin;
extern const int latchPin;
extern const int oePin;

extern const char *ssid;
extern const char *password;