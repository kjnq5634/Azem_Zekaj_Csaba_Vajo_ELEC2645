#include "Attack.h"
#include "Attack.h"
#include <stdint.h>
#include "Boss.h"
#include "InputHandler.h"
#include "LCD.h"
#include "ST7789V2_Driver.h"
#include "stm32l4xx_hal.h"
#include "FightMenu.h"
extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings
extern Boss b; //BossParameters
extern DMG_t d;

void RenderSliderSprite(void){
    LCD_Draw_Rect(10,160,220,70,1,0);}
    //LCD_Draw_Sprite_Scaled(10, 160, 220, 87, undertaleslider_data,1);}
void InitSlider(Slider_t *sli,float speed){
    sli-> height = 60;
    sli-> width = 20;
    sli->x = 110;
    sli->y = 180;
    

    //MOVEMENT SPEED (X AXIS ONLY)
    sli->speedx = speed * 0.5f;


}
void UpdateSlider(Slider_t* sli){
    sli->x += (int16_t)sli->speedx;
    //RIGHT BOUNDARY
     if (sli->x >= 220)
    {
        sli->x = 220; //Anchors the slider
        sli->speedx = -sli->speedx; // at x = 220 the slider then moves moves at an opposite velocity
        //  to what it is now so it 'bounces off the wall of the screen'
    }

    //LEFT BOUNDARY
    if (sli->x <= 0)
    {
        sli->x = 0;
        sli->speedx = -sli->speedx;
    }
}

void DrawAttackSlider(Slider_t *sli,int col){
    LCD_Draw_Rect(
     sli->x,
     sli->y,
     sli->width,
     sli->height, 
     col, 
    1);
}
 void GetDamage(int x, DMG_t *d){
    if (x <40) {d->dmg = 0;}
      else if( x > 41 && x < 110){d->dmg = 50;}
      else if(x>111 && x < 130){d->dmg = 100;}
      else if(x>131 && x<200){d->dmg = 50;}
      else if (x>201 && x<240) {d->dmg = 0;}
      
}
void BossDamage(Boss *b , int damage){
    b->hp -= damage;
}
/*FIGHTSTATE AttackSliderMainFunction(void){
    RenderSliderSprite();
    DrawBossPhase1(&b);
    uint32_t AttackStart = HAL_GetTick();
    InitSlider(&sli,10);
    while (1){
      
     UpdateSlider(&sli);
     DrawAttackSlider(&sli, 3);
     if (current_input.btn2_pressed){
            //sets value of damage based on x
            int dmg = GetDamage(sli.x);
            BossDamage(&b,dmg);
     }

      DrawAttackSlider(&sli, 2);
        LCD_Refresh(&cfg0);
    uint32_t attack_time = HAL_GetTick() - AttackStart;
        if (attack_time < 200) {
            HAL_Delay(200 - attack_time);}
    
}}*/
