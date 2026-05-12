#ifndef GAMESPRITES_H
#define GAMESPRITES_H

#include <stdint.h>
#include "LCD.h"

void Duck_Draw(int16_t x, int16_t y, int8_t facing_right, uint8_t frame);
void Crosshair_Draw(int16_t x, int16_t y, uint8_t empty_ammo);
void Sniper_Draw(int16_t x, int16_t y);
void HitMarker_Draw(int16_t x, int16_t y);

#endif