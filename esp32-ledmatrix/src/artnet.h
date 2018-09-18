#pragma once
#include<stdint.h>

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data);
