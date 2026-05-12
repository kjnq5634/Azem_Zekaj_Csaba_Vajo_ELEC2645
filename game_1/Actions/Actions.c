#include "Actions.h"
#include "FightMenu.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "Boss.h"
#include "Bulb.h"
#include "ST7789V2_Driver.h"
#include "stm32l4xx_hal.h"
#include "Attack.h"
#include <stdint.h>

extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings
extern Boss b; // Boss stats 
extern ItemSelected_t item; //item selection
extern Bulbparams_t BulbParam; //Call bulb parameters (stats)
extern Bulb_t Bulb;//Bulb Movement and Size
extern Slider_t sli; //slider parameters
extern DMG_t d; //Damage variable (global)
#define CAPACITROMAXAMOUNT 5
#define BATTERYMAXAMOUNT 5
static const char* IMenuOption[]={
   "BATTERY",
   " CAPACITOR"
};
// This is the ACT function 
FIGHTSTATE ACTMESSAGE(void){
    //attack menu screen
LCD_Fill_Buffer(0);
//Draw the sprite as a still image
LCD_Draw_Sprite_Scaled(104, 60, 32, 32, ONEwizardbaseform_data, 2);
FIGHTSTATE leave_state = ATKMENU;
while (1) {
     uint32_t act_start = HAL_GetTick();
     
    
    HAL_Delay(500);
    //Drawing the message and correctly initialising the screen
    LCD_Draw_Rect(10,160,220,70,1,0);
    LCD_printString("YOU CANNOT, ", 10, 165, 2, 2);
    LCD_printString(" FOOL YOUR WAY OUT", 10, 175, 2, 2);
    LCD_Refresh(&cfg0);
    // Frame timing - wait for remainder of frame time, also allows to switch to the Boss turn
        uint32_t act_time = HAL_GetTick() - act_start;
        if (act_time < 1000) {
            HAL_Delay(100);
            return leave_state = ATKMENU;        }
}}

//This is the MERCY function this works identically to the ACT function , simply a different message.
FIGHTSTATE MERCYMESSAGE(void){
LCD_Fill_Buffer(0);
LCD_Draw_Sprite_Scaled(104, 60, 32, 32, ONEwizardbaseform_data, 2);
FIGHTSTATE leave_state = ATKMENU;
while (1) {
     uint32_t mercy_start = HAL_GetTick();
     
    
    HAL_Delay(500);
    LCD_Draw_Rect(10,160,220,70,1,0);
    LCD_printString("PEACE ", 10, 165, 2, 2);
    LCD_printString(" IS NEVER AN OPTION", 10, 175, 2, 2);
    LCD_Refresh(&cfg0);
    // Frame timing - wait for remainder of frame time
        uint32_t mercy_time = HAL_GetTick() - mercy_start;
        if (mercy_time < 1000) {
            HAL_Delay(100);
            return leave_state = ATKMENU;        }
}

}
//Item Menu Render:
static void ItemMenuRender(ItemSelected_t *item) {{
    LCD_Fill_Buffer(0);
    
    // Title
    LCD_Draw_Rect(10,160,220,70,1,0);
    
    // Menu itemtions with selection highlight
    for (int i = 0; i < 2; i++) {
        uint16_t Xpos = 15 + (i * 100);
        uint8_t text_size = 2;
        
        if (i == item->itemsel) {
            // Highlight selected  with inverted colors
            // Draw a rectangle around itemselected itemtion
            // We'll use simple marker instead
            LCD_Draw_Rect(Xpos+10,170 , 100, 80, i+2, 3);
        }
        LCD_printString((char*)IMenuOption[i], Xpos+5, 180, 1, text_size);
    }
    BossRenderFunction(&b);
    BulbDrawHealthBar(&BulbParam);
    LCD_Refresh(&cfg0);
}
  }
//Item function initialisation:
void ItemInit(ItemSelected_t *item){
    item -> itemsel = 0;
}
//Item Selection Function
ITEMSTATES ItemSelection(ItemSelected_t *item){
     static Direction last_item = CENTRE;  // Track last direction for debouncing
    ITEMSTATES selected_item = SELECTITEM;  // Which item was selected
    while (1){
     // set time based on built in clock 
      uint32_t item_start = HAL_GetTick();
     //Read current inputs (buttons)
        

     //Read current Joystick directions
      Joystick_Read(&joystick_cfg, &joystick_data);
            Direction current_direction = joystick_data.direction;
            if (current_direction == W && last_item != W) {  // Joystick pushed LEFT
            // Move selection left
            item->itemsel++;
            if (item->itemsel >= 2) {
                item->itemsel = 0;  // Wrap around
            }
        } 
        else if (current_direction == E && last_item != E) {  // Joystick pushed RIGHT
            // Move selection right
            if (item->itemsel == 0) {
                item->itemsel = 2 - 1;  // Wrap around
            } else {
                item->itemsel--;
            }
        }
        
        last_item = current_direction;
    if (current_input.btn2_pressed) {
            // User pressed button - select the highlighted itemtion
            if (item->itemsel == 0) {
                selected_item = BATTERY;
            } else if (item->itemsel == 1) {
                selected_item = CAPACITOR;}

            break;
        }
        
        // Render item
        ItemMenuRender(item);  
        BulbDrawHealthBar(&BulbParam);
      
        // Frame timing - wait for remainder of frame time
        uint32_t item_time = HAL_GetTick() - item_start;
        if (item_time < 200) {
            HAL_Delay(200 - item_time);

        }
    }
    
    return selected_item;
}
  // ITEM (HEALING) Functions:
  ITEMSTATES BatteryFunc(Bulbparams_t *BulbParam){
    // set battery max capacity
    static uint32_t batteryamount = BATTERYMAXAMOUNT;
    uint32_t battery_start = HAL_GetTick();
    while (1){
     
    // after 5 uses the battery item has 0 effect
    if (batteryamount == 0){
        return ATKMENU;
    }
    // Heal bulb ( add health to current health)
    BulbHeal(BulbParam, 20);
    // animation to show healing after bulb is used
    LCD_Draw_Rect(10,160,220,70,2,1);
    LCD_Refresh(&cfg0);
    HAL_Delay(50);
    LCD_Draw_Rect(10,160,220,70,4,1);
    LCD_Refresh(&cfg0);
    HAL_Delay(50);
    LCD_Draw_Rect(10,160,220,70,2,1);
    // reduce battery amount by 1
    batteryamount -- ;
    // render function
    BulbDrawHealthBar(BulbParam);  
     LCD_Refresh(&cfg0); 
   // timer ( ends within 1 second)
    uint32_t batterytime = HAL_GetTick() - battery_start;
    if (batterytime <1000){
        break;
    }
  }}
  // Capacitorworks the same as the Battery function , but more healing and different animation colours
  ITEMSTATES CapacitorFunc(Bulbparams_t *BulbParam){
    uint32_t capacitor_start = HAL_GetTick();
    static uint32_t capacitoramount = CAPACITROMAXAMOUNT;
    while (1){
     
    if(capacitoramount == 0){
        return ATKMENU;
    }
    capacitoramount--;
    BulbHeal(BulbParam, 200);
    LCD_Draw_Rect(10,160,220,70,6,1);
    LCD_Refresh(&cfg0);
    HAL_Delay(50);
    LCD_Draw_Rect(10,160,220,70,5,1);
    LCD_Refresh(&cfg0);
    HAL_Delay(50);
    LCD_Draw_Rect(10,160,220,70,6,1);
    BulbDrawHealthBar(BulbParam);
    LCD_Refresh(&cfg0); 
    
    uint32_t capacitortime = HAL_GetTick() - capacitor_start;
    if (capacitortime <1000){
        break;
    }
  }
  }
//ITEM RUNNING FUNCTION :
FIGHTSTATE ITEMRUN(void){
    ItemInit(&item);
     ITEMSTATES BatteryFunc(Bulbparams_t *BulbParam);
     ITEMSTATES selected_item = SELECTITEM;
          while (1){
             
     //   select between each different item
         switch (selected_item){
          case SELECTITEM: selected_item = ItemSelection(&item);
           break;
          case BATTERY : selected_item = BatteryFunc(&BulbParam);
           return ATKMENU;   
          case CAPACITOR: selected_item = CapacitorFunc(&BulbParam);
           return ATKMENU;
         }
         // BTN based interrupt returns straight back to attack menu
          if (current_input.btn4_pressed)
            {return ATKMENU;}
    }
    }
 
  // Main player attacj function
  FIGHTSTATE AttackFunction(void){
  InitSlider(&sli, 20);
  
    while (1)
    {   
         
        UpdateSlider(&sli);
        //Slider render
        LCD_Fill_Buffer(0);
        DrawAttackSlider(&sli, 1);
        // Draw Slider "Targets"
        LCD_Draw_Rect(0, 180, 40, 70,2, 1);//MISS
        LCD_Draw_Rect(40, 180, 70, 70,6, 1);//MIN DAMAGE
        LCD_Draw_Rect(110, 180, 20, 70,3, 1);//MAX DAMGE
        LCD_Draw_Rect(130, 180, 70, 70,6, 1);//MIN DAMAGE
        LCD_Draw_Rect(200, 180, 40, 70,2, 1);//MISS
        if (current_input.btn2_pressed)
        {   // Damage based on position of slider
            GetDamage(sli.x,&d);
            BossDamage(&b, d.dmg);
             HAL_Delay(300);
             BossAttackFunction(&b,&Bulb,&BulbParam);
             return ATKMENU;
            
            
            
        }
        //render everything required
        BossRenderFunction(&b);
        DrawAttackSlider(&sli, 1);
        BulbDrawHealthBar(&BulbParam);
        LCD_Refresh(&cfg0);
    }
     }
