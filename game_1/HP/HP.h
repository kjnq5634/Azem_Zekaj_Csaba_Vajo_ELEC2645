#ifndef HP_H
#define HP_H



#include <stdint.h>
#include <stdbool.h>
#include "LCD.h"



// Universal entity used for both player and boss
typedef struct {
    int hp;
    int maxHP;
} Entity;


typedef struct {
    uint8_t  x_pos;
    uint8_t y_pos;
    uint8_t width;
    uint8_t height;
    uint8_t colour;
    uint8_t fill;
}HEALTHBAR;
// Initializes an entity with max HP (sets current HP too)
void initEntity(Entity *e, int maxHP);

// Applies damage to any entity
void takeDamage(Entity *e, int damage);
// Applies healing to any entity (in this case it's only for bulb)
void heal(Entity *e, int HealAmount);
// Changes max HP and clamps current HP if needed
void setMaxHP(Entity *e, int maxHP);
void HPBarInit(HEALTHBAR *bar);
// Draws the HP on the LCD
void drawHP (const Entity *e, HEALTHBAR *bar);
//Redraw Healthbar based on new health 
void HPupdate (const Entity *e, HEALTHBAR *bar);



#endif