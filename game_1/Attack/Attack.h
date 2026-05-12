#ifndef ATTACK_H
#define ATTACK_H
#include "Joystick.h"
#include "Utils.h"
#include "Boss.h"
#include "FightMenu.h"
#include <stdint.h>
extern Boss b;
typedef struct{
    int16_t x;
    int16_t y;
    int16_t height;
    int16_t width;
    float speedx;
}Slider_t;
typedef struct{
    int16_t dmg;
}DMG_t;

//FullRenderFunction
void RenderSliderSprite(void);
//Damage Function 
void BossDamage (Boss *b,int damage);
//AttackSlider MAIN FUNCTION:
FIGHTSTATE AttackSliderMainFunction(void);
//Slider Initialisation
void InitSlider(Slider_t *sli,float speed);
void DrawAttackSlider(Slider_t *sli,int col);
//Update Slider Position
void UpdateSlider(Slider_t *sli);
//Positionioning Based Damage
void GetDamage(int x , DMG_t *d);

#endif