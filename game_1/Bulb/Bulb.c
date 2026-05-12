#include "LCD.h"
//#include "UnderzemEngine.h"
#include "Buzzer.h"
#include "Bulb.h"

// Screen dimensions (ST7789V2 display)
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240
#define BALL_RESET_OFFSET 20
#define BUZZER_WALL_FREQ_HZ 1200
#define BUZZER_Bulb_FREQ_HZ 800
#define BUZZER_VOLUME 50
#define BUZZER_BEEP_MS 40
#define BULBMAXHP 500
#define BULBMINDAMAGE 10
// Initialise bulb starting point and movement based parameters
void Bulb_Init(Bulb_t* Bulb, int16_t x, int16_t y, int16_t radius, int16_t speed) {
    Bulb->x = x;
    Bulb->y = y;
    Bulb->speed = speed;
}
//Initialise status parameters for bulb
void BulbParamInit(Bulbparams_t *BulbParam){
    BulbParam -> bulbhp = BULBMAXHP;
    BulbParam -> bulbdamage = BULBMINDAMAGE;
}
//Global Healing Function for my bulb (player)
void BulbHeal (Bulbparams_t *BulbParam, int healamount){
      //Clamps HP to MAXIMUM so it can not be exceeded
     if (healamount + BulbParam -> bulbhp >= BULBMAXHP){
        BulbParam -> bulbhp = BULBMAXHP;
     }
      else {
        BulbParam -> bulbhp = BulbParam -> bulbhp + healamount ;

      }
}
// bulb damage function (works the same as the global function removes damage based on health pointer)
void BulbTakeDamage(Bulbparams_t *BulbParam,int damage){
    if(damage >= BulbParam -> bulbhp){
        BulbParam -> bulbhp = 0;
    }
    else{
        BulbParam -> bulbhp -= damage;
    }
}
// updates bulb location
void Bulb_Update(Bulb_t* Bulb, Joystick_t *joystick_data)
{
    int16_t dx = 0;
    int16_t dy = 0;
  switch (joystick_data -> direction)
    {
        case N:  dy = -Bulb->speed; break;
        case NE: dy = -Bulb->speed; dx =  Bulb->speed; break;
        case E:  dx =  Bulb->speed; break;
        case SE: dy =  Bulb->speed; dx =  Bulb->speed; break;
        case S:  dy =  Bulb->speed; break;
        case SW: dy =  Bulb->speed; dx = -Bulb->speed; break;
        case W:  dx = -Bulb->speed; break;
        case NW: dy = -Bulb->speed; dx = -Bulb->speed; break;
        default: break;
    }

    Bulb->x += dx;
    Bulb->y += dy;
}
// draws bulb sprite
void Bulb_Draw(Bulb_t*Bulb){
    LCD_Draw_Sprite_Scaled(
        Bulb->x,
        Bulb->y,
        14,
        12,
        bulb_data,
        1

    );
}
// bulb death sprite
void Bulb_damaged(Bulb_t*Bulb){
    LCD_Draw_Sprite_Scaled(
        Bulb->x,
        Bulb->y,
        14, 
        12, 
        bulbdamage_data, 
        1);
}
// Draw bulb health bar
void BulbDrawHealthBar(Bulbparams_t *BulbParam){
    int width = BulbParam->bulbhp/5; // width based on bulb hp

         if( BulbParam->bulbhp <= 0 ){
           LCD_Draw_Rect(80, 150, 100, 10, 1, 0); //healthbar border
           LCD_Draw_Rect(80, 150, width, 10, 0, 1);} //sets health bar visually to 0 ( black )
           else {
            LCD_Draw_Rect(80, 150, 100, 10, 1, 0); // healthbar border
            LCD_Draw_Rect(80, 150, width, 10, 6, 1); // changed the width of the health bar (colour) based on hp
}}
// clamps bulb  to one location
void Bulb_Clamp(Bulb_t* Bulb)
{   // when bulb exceeds boundary bulb x / y position returns to that boundary
    if (Bulb->x >= 240)
        Bulb->x = 240;
    else if (Bulb->x <= 10)
        Bulb->x = 10;

    if (Bulb->y <= 160)
        Bulb->y = 160;
    else if (Bulb->y >= 230)
        Bulb->y = 230;
}

Position2D Bulb_GetPos(Bulb_t* Bulb) {
    //position pointers
    Position2D pos;
    pos.x = Bulb->x; 
    pos.y = Bulb->y;
    return pos;
}

int16_t Bulb_GetRadius(Bulb_t* Bulb) {
    //radius pointers
    return Bulb->radius;
}
