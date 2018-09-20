#include "config.h"

const int colorPins[NUM_COLOR_PINS] = {13, 12, 14, 27, 26, 25, 2, 4};

const int panelMapping[PANELS_X][PANELS_Y] = {{0, 1}, {2, 3}};
//const int panelMapping[PANELS_X][PANELS_Y] = {{0, 1}};

const int clkPin = 18;
const int latchPin = 19;
const int oePin = 23;

const char *ssid = "mrmcd2018";
const char *password = "mrmcd2018";