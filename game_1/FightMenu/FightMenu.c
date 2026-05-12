#include "FightMenu.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include "Boss.h"
#include "Music.h"
#include "GameStates.h"
extern ST7789V2_cfg_t cfg0;  // LCD configuration from main.c
extern Joystick_cfg_t joystick_cfg;  // Joystick configuration
extern Joystick_t joystick_data;     // Current joystick readings
extern Boss b;
extern Bulbparams_t BulbParam;
//char ACTSTRING[] = {"YOU CANNOT FOOL YOUR WAY OUT OF THIS"};
static const char* MenuOption[]={
   "FIGHT",
   " ITEM ",
   " MERCY ",
   " ACT"
};
#define NUM_FMENU_OPTIONS 4
#define MENU_FRAME_TIME_MS 30

static void FightMenuRender(OptionSelection_t *op) {
    LCD_Fill_Buffer(0);
    
    // Title
    LCD_Draw_Rect(10,160,220,70,1,0);
    BulbDrawHealthBar(&BulbParam);
    
    // Menu options with selection highlight
    for (int i = 0; i < NUM_FMENU_OPTIONS; i++) {
        uint16_t Xpos = 15 + (i * 40);
        uint8_t text_size = 1;
        
        if (i == op->selected) {
            // Highlight selected option with inverted colors
            // Draw a rectangle around selected option
            // We'll use simple marker instead
            LCD_Draw_Rect(Xpos+10,170 , 40, 30, i+2, 1);
        }
        LCD_printString((char*)MenuOption[i], Xpos+15, 180, 1, text_size);
    }
          BossRenderFunction(&b);
         
    // Instructions
    LCD_printString("Press BT4", 50, 240, 1, 1);
    
    LCD_Refresh(&cfg0);
}
void FightMenuInit(OptionSelection_t *op) {
    op->selected = 0;
}
FIGHTSTATE FightMenuUpdate(OptionSelection_t *op){
     static Direction last_direction = CENTRE;  // Track last direction for debouncing
    FIGHTSTATE selected_state = ATKMENU;  // Which game was selected
        uint32_t frame_start = HAL_GetTick();
        
        // Read input
        Input_Read();
        
        // Read current joystick position
        Joystick_Read(&joystick_cfg, &joystick_data);
        // Handle joystick navigation (up/down to select option)
        Direction current_direction = joystick_data.direction;
            if (current_direction == W && last_direction != W) {  // Joystick pushed LEFT
            // Move selection left
            op->selected++;
            if (op->selected >= NUM_FMENU_OPTIONS) {
                op->selected = 0;  // Wrap around
            }
        } 
        else if (current_direction == E && last_direction != E) {  // Joystick pushed RIGHT
            // Move selection right
            if (op->selected == 0) {
                op->selected = NUM_FMENU_OPTIONS - 1;  // Wrap around
            } else {
                op->selected--;
            }
        }
        
        last_direction = current_direction;
 if (current_input.btn2_pressed) {
            // User pressed button - select the highlighted option
            if (op->selected == 0) {
                selected_state = FIGHT;
            } else if (op->selected == 1) {
                selected_state = ITEM;
            } else if (op->selected == 2) {
                selected_state = MERCY;
            }else if (op->selected == 3) {
                selected_state = ACT;
            }
        }
        
        // Render op and Bulb health bar
        FightMenuRender(op); 
        BulbDrawHealthBar(&BulbParam);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < MENU_FRAME_TIME_MS) {
            HAL_Delay(MENU_FRAME_TIME_MS - frame_time);
        }
    
    return selected_state;  // Return which game was selected

}