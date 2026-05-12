/**
 * @file DuckEngine.c
 * @brief Duck Hunt engine implementation
 */

#include "DuckEngine.h"
#include "Background.h"
#include "GameSprites.h"
#include "LCD.h"
#include "Buzzer.h"
#include "main.h"
#include "rng.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"

#include <stdint.h>

#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  240

//duck sprites are 8 by 8 and scaled up by 3 when drawn
#define DUCK_SCALE     3
#define DUCK_W (8 * DUCK_SCALE)
#define DUCK_H (8 * DUCK_SCALE)

#define CROSSHAIR_SPEED 5 //crosshair movement speed per frame

#define MAX_LIVES 10
#define MAX_AMMO 6

//buzzer frequences for different events
#define BUZZER_EMPTY_FREQ_HZ 200
#define BUZZER_RELOAD_FREQ_HZ 900
#define BUZZER_FIRE_FREQ_HZ   1400
#define BUZZER_HIT_FREQ_HZ    2200
#define BUZZER_ESCAPE_FREQ_HZ 400
#define BUZZER_VOLUME         50
#define BUZZER_BEEP_MS        40

extern Buzzer_cfg_t buzzer_cfg;
extern RNG_HandleTypeDef hrng;

static uint32_t buzzer_stop_tick = 0;
static uint32_t rng_state = 1;

static uint8_t duck_speed_multiplier = 1;

static void DuckEngine_Beep(uint32_t freq_hz) //starts short buzzer for game event
{
    buzzer_tone(&buzzer_cfg, freq_hz, BUZZER_VOLUME);
    buzzer_stop_tick = HAL_GetTick() + BUZZER_BEEP_MS;
}

static void DuckEngine_UpdateBuzzer(void) //stop buzzer once BEEP time passed
{
    if (buzzer_stop_tick != 0 && (int32_t)(HAL_GetTick() - buzzer_stop_tick) >= 0) {
        buzzer_off(&buzzer_cfg);
        buzzer_stop_tick = 0;
    }
}

//used to stop crosshair and ducks from going outside allowed range of areas
static int16_t clamp_i16(int16_t v, int16_t lo, int16_t hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

//draws filled rectangle clipped to screen boundaries so ducks can't move off screen
static void LCD_Draw_Rect_Clipped(int16_t x,int16_t y,int16_t width,int16_t height,uint8_t colour)
{
    if (width <= 0 || height <= 0) { //ignores rectangles of no size
        return;
    }
    //rectangle corners
    int16_t x0 = x; 
    int16_t y0 = y; 
    int16_t x1 = x + width - 1;
    int16_t y1 = y + height - 1;

    if (x1 < 0 || y1 < 0 || x0 >= SCREEN_WIDTH || y0 >= SCREEN_HEIGHT) { //if rectangle out of screen don't draw it
        return;
    }
    //clips edges inside LCD screen
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
    if (y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;

    LCD_Draw_Rect((uint16_t)x0,(uint16_t)y0,(uint16_t)(x1 - x0 + 1),(uint16_t)(y1 - y0 + 1),colour,1);
}

static void seed_fast_rng(void) //random number generator using STM32 hardware and if RNG fails Hal_GetTick is used
{
    uint32_t seed = 1;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &seed) != HAL_OK) {
        seed = HAL_GetTick();
    }
    if (seed == 0) {
        seed = 1;
    }
    rng_state = seed;
}

static uint32_t fast_rand(void) //simple fast RNG
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static uint32_t rand_range(uint32_t max) //return random number from 0 to max-1
{
    if (max == 0) {
        return 0;
    }
    return fast_rand() % max;
}

//checks whether crosshair is in duck's hitbox, AABB collision detection
static uint8_t point_in_duck(int16_t px, int16_t py, Duck_t* duck)
{
    if (!duck->alive) return 0;

    return (px >= duck->x &&
            px <= (duck->x + DUCK_W) &&
            py >= duck->y &&
            py <= (duck->y + DUCK_H));
}

//spawn duck outside of left/right side of screen, height and speed randomised
static void spawn_duck(Duck_t* duck)
{
    uint8_t from_left = (uint8_t)rand_range(2);

    duck->alive = 1;
    duck->y = 35 + (int16_t)rand_range(100); //random sky height
    duck->frame = 0;
    duck->hit_timer = 0;
    duck->bob_dir = 1; //start bobbing downwards

    if (from_left) {
        duck->x = -DUCK_W;
        duck->vx = 2 + (int16_t)rand_range(3); //move right w speed 2 to 4
    } else {
        duck->x = SCREEN_WIDTH + DUCK_W;
        duck->vx = -(2 + (int16_t)rand_range(3));   //move left with speed -2 to -4
    }
}

//move crosshair based on joystick input, X normally, Y is inverted
static void update_crosshair(DuckEngine_t* engine, Joystick_t* joy)
{
    //coord_mapped.x is -1 left to +1 right
    //coord_mapped.y is -1 down to +1 up
    engine->crosshair_x += (int16_t)(joy->coord_mapped.x * CROSSHAIR_SPEED);
    engine->crosshair_y -= (int16_t)(joy->coord_mapped.y * CROSSHAIR_SPEED);

    //keep crosshair in sky
    engine->crosshair_x = clamp_i16(engine->crosshair_x, 10, SCREEN_WIDTH - 10);
    engine->crosshair_y = clamp_i16(engine->crosshair_y, 24, 180);
}

static void update_ducks(DuckEngine_t* engine) //update all ducks movement, bobbing anim, respawn, life loss
{
    for (uint8_t i = 0; i < DUCK_MAX_COUNT; i++) {
        Duck_t* d = &engine->ducks[i];
    
        if (!d->alive) { //if this duck isn't alive then randomly respawn it
            if (rand_range(100) < 3) {
                spawn_duck(d);
            }
            continue;
        }

        //move duck horizontally
        d->x += d->vx * duck_speed_multiplier; //multiplier of 0 or 1 for night/day

        if(duck_speed_multiplier == 0) {
            if ((d->frame % 2) == 0) { //duck moves every even framse
                d->x += d->vx;
            }
            else {
                d->x += d->vx * duck_speed_multiplier;
            }
        }

        if (d->bob_dir > 0) { //duck bobbing in sky
            d->y += 1;
            if ((d->frame % 4) == 3) { //every 4th frame bobbing changes direction
                d->bob_dir = -1;
            }
        } else {
            d->y -= 1;
            if ((d->frame % 4) == 3) { 
                d->bob_dir = 1;
            }
        }
        d->y = clamp_i16(d->y,35,155); //keep ducks in sky area 
        d->frame++;

        if (d->x > SCREEN_WIDTH + DUCK_W || d->x < -DUCK_W - 4) { //if duck escapes, lose a life
            d->alive = 0;

            if (engine->lives > 0) {
                engine->lives--;
                DuckEngine_Beep(BUZZER_ESCAPE_FREQ_HZ);
            }

            
        }
    }
}

//reload ammo when reload button pressed, prevent long button press from constant reload
static void handle_reload(DuckEngine_t* engine, uint8_t reload_pressed) {
    if (reload_pressed && !engine->reload) {
        engine->reload = 1;
        engine->ammo = MAX_AMMO;
        DuckEngine_Beep(BUZZER_RELOAD_FREQ_HZ);
    }
    if (!reload_pressed) {
        engine->reload = 0;
    }
}
//handle shooting input, ammo, hit detection, score, shot sounds
static void handle_fire(DuckEngine_t* engine, uint8_t fire_pressed)
{
    if (fire_pressed && !engine->fire_pressed) { //fire once per button press
        engine->fire_pressed = 1;

        if(engine->ammo == 0) {
            DuckEngine_Beep(BUZZER_EMPTY_FREQ_HZ);
            return;
        }
        engine->ammo--; //use one bullet
        DuckEngine_Beep(BUZZER_FIRE_FREQ_HZ); //play firing sound

        for (uint8_t i = 0; i < DUCK_MAX_COUNT; i++) {
            Duck_t* d = &engine->ducks[i];

            //check each duck whether crosshair hit it
            if (point_in_duck(engine->crosshair_x, engine->crosshair_y, d)) {
                d->alive = 0;
                d->hit_timer = 8;
                engine->old_duck_alive[i] = 1;
                engine->score++;
                DuckEngine_Beep(BUZZER_HIT_FREQ_HZ);
                break;
            }
        }
    }

    if (!fire_pressed) { //reset fire once button released
        engine->fire_pressed = 0;
    }
}

void DuckEngine_Init(DuckEngine_t* engine) //initiliase game engine state at start of a new game
{
    seed_fast_rng();

    //reset main gameplay variables
    engine->score = 0;
    engine->lives = MAX_LIVES;
    engine->crosshair_x = SCREEN_WIDTH / 2;
    engine->crosshair_y = SCREEN_HEIGHT / 2;
    engine->old_crosshair_x = engine->crosshair_x;
    engine->old_crosshair_y = engine->crosshair_y;
    engine->fire_pressed = 0;
    engine->ammo = MAX_AMMO;
    engine->reload = 0;

    //clear all ducks and old drawing positions
    for (uint8_t i = 0; i < DUCK_MAX_COUNT; i++) {
        engine->ducks[i].alive = 0;
        engine->ducks[i].x = 0;
        engine->ducks[i].y = 0;
        engine->ducks[i].vx = 0;
        engine->ducks[i].bob_dir = 1;
        engine->ducks[i].frame = 0;
        engine->ducks[i].hit_timer = 0;

        engine->old_duck_x[i] = 0;
        engine->old_duck_y[i] = 0;
        engine->old_duck_alive[i] = 0;
    }

    //start game with two ducks active
    spawn_duck(&engine->ducks[0]);
    spawn_duck(&engine->ducks[1]);
}

//main engine update function called once per frame in my main
//updates movement, shooting, reload, duck, lives, ammo, sound
uint8_t DuckEngine_Update(DuckEngine_t* engine, Joystick_t* joy, uint8_t fire_pressed, uint8_t reload_pressed)
{
    update_crosshair(engine, joy);
    update_ducks(engine);
    handle_reload(engine,reload_pressed);
    handle_fire(engine, fire_pressed);

    DuckEngine_UpdateBuzzer(); //stop buzzer after their timer expires
    return engine->lives; //return lives so main game can detect game over
}

//draw current frame, old moving objects erased by sky colour then new are drawn at new position
void DuckEngine_Draw(DuckEngine_t* engine, uint8_t sky_colour, uint8_t night_mode)
{
    //ducks are restricted to the sky area so this doesn't erase the grass area
    for (uint8_t i = 0; i < DUCK_MAX_COUNT; i++) {
        if (engine->old_duck_alive[i]) {
            LCD_Draw_Rect_Clipped(engine->old_duck_x[i] - 2,
                                    engine->old_duck_y[i] - 2,
                                    DUCK_W + 4,
                                    DUCK_H + 4,
                                    sky_colour);
        }
    }

    //erase old crosshair
    LCD_Draw_Rect_Clipped(engine->old_crosshair_x - 11,
                          engine->old_crosshair_y - 11,
                          22,
                          22,
                          sky_colour);

    // draw ducks at their new positions
    for (uint8_t i = 0; i < DUCK_MAX_COUNT; i++) {
        Duck_t* d = &engine->ducks[i];

        if (!d->alive) { //if duck was just hit, draw temporary hit marker at old position
            if(d->hit_timer > 0) {
                HitMarker_Draw(engine->old_duck_x[i] + DUCK_W / 2, engine->old_duck_y[i] + DUCK_H /2);
                d->hit_timer--;
                engine->old_duck_alive[i] = 1;
            } else {
                engine->old_duck_alive[i] = 0;
            }
            continue;
        }

        uint8_t anim_frame = (d->frame / 6) % 2; //alternate between two duck sprites for wing animation
        int8_t facing_right = (d->vx > 0) ? 1 : 0; //flip sprite depending on movement direction

        Duck_Draw(d->x, d->y, facing_right, anim_frame);

        engine->old_duck_x[i] = d->x;
        engine->old_duck_y[i] = d->y;
        engine->old_duck_alive[i] = 1;
    }

    // draw crosshair last so appears over everything, crosshair changes colour when ammo empty
    Crosshair_Draw(engine->crosshair_x, engine->crosshair_y,engine-> ammo==0);

    engine->old_crosshair_x = engine->crosshair_x;
    engine->old_crosshair_y = engine->crosshair_y;
}

uint8_t DuckEngine_GetLives(DuckEngine_t* engine) //return lives remaining for the HUD
{
    return engine->lives;
}

uint16_t DuckEngine_GetScore(DuckEngine_t* engine) //return current score for HUD too
{
    return engine->score;
}

//set duck movement speed, 0 is night time, 1 is normal daytime speed
void DuckEngine_SetSpeedMultiplier(DuckEngine_t *engine, uint8_t multiplier) {
    duck_speed_multiplier = multiplier;
}
//return current ammo count for ammo displayed
uint8_t DuckEngine_GetAmmo(DuckEngine_t* engine) {
    return engine->ammo;
}