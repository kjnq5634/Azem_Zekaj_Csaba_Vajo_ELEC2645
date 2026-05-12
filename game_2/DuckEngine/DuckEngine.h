/**
 * @file DuckEngine.h
 * @brief Duck Hunt engine
 */

#ifndef DUCKENGINE_H
#define DUCKENGINE_H

#include <stdint.h>
#include "Joystick.h"

#define DUCK_MAX_COUNT 3

typedef struct {
    int16_t x;
    int16_t y;
    int16_t vx;
    int8_t bob_dir;
    uint8_t alive;
    uint8_t frame;
    uint8_t hit_timer;
    //DuckType_t type;
} Duck_t;

typedef struct {
    Duck_t ducks[DUCK_MAX_COUNT];

    int16_t crosshair_x;
    int16_t crosshair_y;

    int16_t old_crosshair_x;
    int16_t old_crosshair_y;

    int16_t old_duck_x[DUCK_MAX_COUNT];
    int16_t old_duck_y[DUCK_MAX_COUNT];
    uint8_t old_duck_alive[DUCK_MAX_COUNT];

    uint16_t score;
    uint8_t lives;
    uint8_t fire_pressed;
    uint8_t ammo;
    uint8_t reload;
} DuckEngine_t;

typedef enum {
    DUCK_NORMAL = 0,
    DUCK_GOLD = 1
} DuckType_t;

void DuckEngine_Init(DuckEngine_t* engine);
uint8_t DuckEngine_Update(DuckEngine_t* engine, Joystick_t* joy, uint8_t fire_pressed, uint8_t reload_pressed);
void DuckEngine_Draw(DuckEngine_t* engine, uint8_t sky_colour, uint8_t night_mode);
uint8_t DuckEngine_GetLives(DuckEngine_t* engine);
uint16_t DuckEngine_GetScore(DuckEngine_t* engine);
void DuckEngine_SetSpeedMultiplier(DuckEngine_t* engine, uint8_t multiplier);
uint8_t DuckEngine_GetAmmo(DuckEngine_t* engine);

#endif