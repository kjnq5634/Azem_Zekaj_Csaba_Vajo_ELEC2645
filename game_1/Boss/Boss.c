#include "Boss.h"


#include "FightMenu.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "LCD.h"
#include "Music.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "Bulb.h"
#include "HP.h"
#include "stm32l4xx_hal_flash.h"
#include "Hitbox.h"


#define BOSSANIMATIONFRAME 100
extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Bulb_t Bulb; // Bulb postioning
extern Bulbparams_t BulbParam; //Bulb stats
extern Joystick_cfg_t joystick_cfg; //JoystickConfiguration
extern Joystick_t joystick_data;// Joystick data struct

#define BOSSMAXHP 200
//Boss initialisation
void BossInit(Boss *b){
     b->hp = BOSSMAXHP ; 
     b->dmg = 10;
}
// Boss health drawing function
void DrawBossHP(Boss *b)
       {
         int width = b->hp / 2; //set healthbar current width

         if( b->hp <= 0 ){
           LCD_Draw_Rect(80, 30, 100, 10, 1, 0); //white border
           LCD_Draw_Rect(80, 30, width, 10, 0, 1);} //health bar = black when health is 0
           else {
            LCD_Draw_Rect(80, 30, 100, 10, 1, 0); //white border
            LCD_Draw_Rect(80, 30, width, 10, 2, 1); // width changes based on hp value
           }
         }
       
      
   // This is the initial boss drawing function
 void DrawBossPhase1(Boss *b){
     // timer for boss frame animation
       uint32_t frametimer = HAL_GetTick() - b->phaseStartTimer;
      // draws frame 1 for 1 second , frame 2 for 300 seconds between 200 and 500 ms 
      if(frametimer < 200){
        LCD_Draw_Sprite_Scaled(104, 60, 32, 32, ONEwizardbaseform_data, 2);
        }
      
       else if(frametimer >200 && frametimer < 500 ){

        LCD_Draw_Sprite_Scaled(104, 60, 32, 32, TWOwizardbaseform_data, 2);
         } 
         else {
        // loop animation ( resets timer back to 0)
        b->phaseStartTimer = HAL_GetTick();
    }
    // play start music
   startmusic();
 } 
 FIGHTSTATE BossAttackPhaseOne(Boss *b,Bulb_t *Bulb){
   // Hit box for maze walls
   static HITBOX dangerZones[]= {
    {20,160,40,5},
    {60,160,20,10},
    {80,160,100,10},
    {180,160,20,10},
    {20,220,70,20},
    {70,190,20,40},
    {180,190,40,40},
    {200,190,20,40}
   };
   // initial timer
   uint32_t FightTimer = HAL_GetTick();

    // Invincibility timer
    uint32_t lastHitTime = 0;
    const uint32_t iFrameDuration = 200;

   while (1){
    // Read Joystick movement
   Joystick_Read(&joystick_cfg, &joystick_data);
   LCD_Fill_Buffer(0); //clear screen
   Bulb_Draw(Bulb); // render bulb
   LCD_Draw_Rect(10,160,220,70,1,0); // draw attack / bulb border
   Bulb_GetPos(Bulb); // Bulb x and y
   Bulb_GetRadius(Bulb); //Bulb size
   Bulb_Clamp(Bulb); // Clamp bulb betwee 240 >= x >= 10 and 240 >= y >= 160
   Bulb_Update(Bulb,&joystick_data); //update bulb location based on joystick
    HITBOX bulbbox = {Bulb->x,Bulb->y,1,1}; // set bulb hitbox
   for (int i = 0; i<8 ; i ++){
   if (CheckCollision(dangerZones[i], bulbbox)) //create separate hitbox for ever rectangle in maze (9 retangles so counts 0 - 8)
            {
                // I-frame clock
                if (HAL_GetTick() - lastHitTime >= iFrameDuration)
                {
                    BulbTakeDamage(&BulbParam, 1);
                    lastHitTime = HAL_GetTick(); // little break for dmg
                }
                 
                break;
            }
     }
    for (int j = 0; j<8; j ++){
      // draw 9 rects / draw maze
      LCD_Draw_Rect(dangerZones[j].x,dangerZones[j].y,dangerZones[j].w,dangerZones[j].h,1,1);} 
// 10 second fight timer over ridden by reaching end of maze
    uint32_t FightLivetime = HAL_GetTick() - FightTimer;
    if(FightLivetime > 10000){
      if(Bulb->x <= 210){
        BulbParam.bulbhp -= 1;
        return ATKMENU;
      }
    }else if(Bulb->x >= 210){
      return ATKMENU;
    }
    // rendering functions
    DrawBossPhase1(b);
    DrawBossHP(b);
    BulbDrawHealthBar(&BulbParam);
    LCD_Refresh(&cfg0);
   }}

   FIGHTSTATE BossAttackPhaseTwo(Boss *b, Bulb_t *Bulb){
    Bullet bullets[20];
    for (int i=0;i<20;i++){
      bullets[i].x = 110;
      bullets[i].y = 180;
      bullets[i].angle = i+0.3f;
      bullets[i].speed = 1.5f;
      bullets[i].active = 1;
    } // set an array for bullets struct is based on angles
    uint32_t BulletStart = HAL_GetTick();
    while(1){
      uint32_t BulletCurrent = HAL_GetTick(); //bullet timer
      Joystick_Read(&joystick_cfg,&joystick_data );
      LCD_Fill_Buffer(0); //clear screen
      Bulb_GetPos(Bulb); 
      Bulb_Clamp(Bulb); // Bulb initialisation
      Bulb_Update(Bulb, &joystick_data); //bulb location based on joystick
      HITBOX Bulbhb = {Bulb->x,Bulb->y,8,8}; // bulb hitbox
      for (int j = 0; j<20; j++){
        // draw bullets
        if(!bullets->active) continue;
        bullets[j].angle += 0.05f; // to create a spiral of bullets
        bullets[j].x += cosf(bullets[j].angle) * bullets[j].speed;
        bullets[j].y += sinf(bullets[j].angle) * bullets[j].speed; // trigonometric functions allow the circle to pulse
        LCD_Draw_Rect(bullets[j].x,bullets[j].y,3,3,6,1); // bullet drawing function
        HITBOX bullethb = {bullets[j].x,bullets[j].y,3,3}; // bullet hitbox 
        if (CheckCollision(Bulbhb,bullethb)){
          BulbTakeDamage(&BulbParam, 5); // damage bulb
          bullets[j].active = 0; // deactivate bullets upon hit
          Bulb_damaged(Bulb); // bulb damage sprite
          LCD_Refresh(&cfg0);
        }
        if(bullets[j].y>=240){
          bullets[j].active = 0;
        } // deactivate bullets at bottom of screen
      }
      // main render functions
      Bulb_Draw(Bulb);
      BulbDrawHealthBar(&BulbParam);
      DrawBossHP(b);
      LCD_Draw_Rect(10,160,220,70,1,0);
      DrawBossPhase2(b);
      //attack timer
      LCD_Refresh(&cfg0);
      if(BulletCurrent-BulletStart >10000){
        return ATKMENU;
      }

    }
   }
// second phase boss render func
void DrawBossPhase2(Boss *b){
       uint32_t frametimer = HAL_GetTick() - b->phaseStartTimer;
       //animation works the same as the 1st function but this has 3 frams and so 3 different ranges
      if(frametimer < 200){

        LCD_Draw_Sprite_Scaled(104, 60, 32, 32, finalform_data1, 2);
        //HAL_Delay(50);
        }
      
       else if(frametimer > 200 &&frametimer < 500 ){

        LCD_Draw_Sprite_Scaled(104, 60, 32, 32, finalform_data2, 2);
        }
        else if (frametimer < 500 && frametimer < 800){

         LCD_Draw_Sprite_Scaled(104, 60, 32, 32, finalform_data2, 2);
        }
        else {
        b->phaseStartTimer = HAL_GetTick(); // loop animation
    }

 } 
 //Boss death function ( overrides current state of boss hp = 0)
 FIGHTSTATE BossDeath(Boss *b){
  LCD_Fill_Buffer(0);
  LCD_printString("Victory!!!!", 30, 120,6, 2);
        LCD_printString("Press RESET to exit",40,160,1,1);
        LCD_Refresh(&cfg0);
 }
 // Character death function over rides current state if player hp = 0 
 FIGHTSTATE CharacterDeath(Bulbparams_t *BulbParam){
    LCD_Fill_Buffer(0);
    LCD_printString("YOU DIED",90,120,2,2);
    LCD_Draw_Sprite_Scaled(120, 90, 14, 12, bulbdamage_data, 1);
    LCD_printString("Press RESET btn to return",90,160,1,1);
    LCD_Refresh(&cfg0);
  }
  // Phase based render function
  PHASE BossRenderFunction(Boss *b){
    static PHASE previousstate = PHASE1;
    // Health based phase changes as you can see with the ranges below phase changes at half health and death occurs at 0
    int Health = b->hp / 2;
    PHASE BossState  = PHASE1;
    DrawBossHP(b);
    if (Health >=50){
      BossState = PHASE1;
    }
    else if (Health <49 && Health > 1){
      BossState = PHASE2;
    }
    else if (Health <= 0){
      BossState = DEATH;
    }
    //State Based timer for boss animation
    if (BossState != previousstate){
      b-> phaseStartTimer -= HAL_GetTick();
      previousstate = BossState;
    }
    // switch statement based on hp calls drawing functions based on PHASE
    switch (BossState){
      case PHASE1 :
      DrawBossPhase1(b);
      break;
      case PHASE2:
      DrawBossPhase2(b);
      break;
      case DEATH:
      break;
      case CHARACTERLOSS:
      break;
    }

  }
  //Fighstate based function 
  FIGHTSTATE BossAttackFunction(Boss *b,Bulb_t *Bulb,Bulbparams_t *BulbParam){
    Bulb_Init(Bulb, 20, 200, 1, 4);
    // much like the render function this calls different attacks and states based on the boss state and character state
    int HealthPoints = b-> hp/2;
    PHASE BossStateFight = PHASE1;
    if(HealthPoints>=50){
      BossStateFight = PHASE1;
    }else if (HealthPoints <49 && HealthPoints > 1){
      BossStateFight = PHASE2;
    }else if (HealthPoints< 1){
      BossStateFight = DEATH;
    }
    if(BulbParam->bulbhp <= 0){
      BossStateFight = CHARACTERLOSS;
    }
  switch (BossStateFight){
    case PHASE1: BossAttackPhaseOne(b,Bulb); // Boss 1st attack (1st phase only)
    break;
    case PHASE2: BossAttackPhaseTwo(b,Bulb); // Boss 2nd attack (2nd phase only)
    break;
    case DEATH: BossDeath(b); // victory screen based on boss death
    break;
    case CHARACTERLOSS: CharacterDeath(BulbParam); // loss screen based on player death
    break;

  }
  }