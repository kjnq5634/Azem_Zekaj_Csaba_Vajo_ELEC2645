#ifndef BULB_H
#define BULB_H

#include "Joystick.h"
#include "gpio.h"
#include <stdint.h>
#include "Utils.h"
// BULB SPRITES
static const uint8_t bulb_data[168] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 
0, 0, 0, 6, 6, 6, 6, 6, 6, 0, 0, 0, 
0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 
0, 6, 6, 6, 0, 6, 6, 0, 0, 6, 6, 0, 
0, 6, 6, 6, 0, 0, 6, 0, 6, 6, 6, 0, 
0, 0, 6, 6, 6, 0, 0, 0, 6, 6, 0, 0, 
0, 0, 6, 6, 6, 0, 0, 6, 6, 6, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
static const uint8_t bulbdamage_data[168] = {

0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 
0, 0, 0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 
0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 
0, 2, 2, 2, 0, 2, 2, 0, 0, 2, 2, 0, 
0, 2, 2, 2, 0, 0, 2, 0, 2, 2, 2, 0, 
0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 0, 0, 
0, 0, 2, 2, 2, 0, 0, 2, 2, 2, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

typedef struct {
    int16_t x;        // Bulb X position (left edge)
    int16_t y;        // Bulb Y position (top edge)
    int16_t radius;  // Bulb radius (placeholder)
    int16_t speed;    // Movement speed (pixels per frame)
    uint16_t score;   // Game score (incremented on successful hit)
} Bulb_t;
typedef struct {
    int bulbhp; 
    int bulbdamage;
}Bulbparams_t; //Bulb health parameters
void Bulb_Init(Bulb_t* Bulb, int16_t x, int16_t y, int16_t radius, int16_t speed); //initialise bulb position and size
void Bulb_Update(Bulb_t* Bulb, Joystick_t *joystick_data); // update position
void Bulb_Draw(Bulb_t* Bulb); //Draws bulb sprite
void Bulb_damaged(Bulb_t* Bulb); // Death screen bulb
void Bulb_Clamp(Bulb_t* Bulb); //clamps bulb to a box
AABB Bulb_GetAABB(Bulb_t* Bulb);
void BulbParamInit(Bulbparams_t *BulbParam); // initialise bulb hp
void BulbDrawHealthBar(Bulbparams_t *BulbParam); // bulb health drawing function
void BulbHeal (Bulbparams_t *BulbParam , int healamount); // personal healing function
void BulbTakeDamage(Bulbparams_t *BulbParam,int damage); // personal damage function
Position2D Bulb_GetPos(Bulb_t* Bulb); // gets the 2D position of Bulb (for hitbox)
int16_t Bulb_GetRadius(Bulb_t* Bulb); // gets radius for bulb (for hitbox)



#endif // Bulb_H