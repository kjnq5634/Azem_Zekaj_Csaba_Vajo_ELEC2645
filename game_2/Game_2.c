#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;  // Buzzer control

/**
 * @brief Game 2 Implementation - Student can modify
 * 
 * EXAMPLE: Shows how to use the Buzzer for sound effects
 * This is a placeholder with a bouncing animation.
 * Replace this with your actual game logic!
 */

// Game state - customize for your game
static uint32_t animation_counter = 0;
static int16_t moving_y = 0;
static int8_t move_direction = 1;

// Frame rate for this game (in milliseconds) - runs slower than Game 1
#define GAME2_FRAME_TIME_MS 50  // ~20 FPS (different from Game 1!)

MenuState Game2_Run(void) {
    // Initialize game state
    animation_counter = 0;
    moving_y = 0;
    move_direction = 1;
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1200, 30);  // 1.2kHz at 30% volume
    HAL_Delay(50);  // Brief beep duration
    buzzer_off(&buzzer_cfg);  // Stop the buzzer
    
    MenuState exit_state = MENU_STATE_HOME;  // Default: return to menu
    
    // Game's own loop - runs until exit condition
    while (1) {
        uint32_t frame_start = HAL_GetTick();
        
        // Read input
        Input_Read();
        
        // Check if button was pressed to return to menu
        if (current_input.btn3_pressed) {
            exit_state = MENU_STATE_HOME;
            break;  // Exit game loop
        }
        
        // UPDATE: Game logic
        animation_counter++;
        
        // Simple animation: move object up and down
        moving_y += move_direction * 2;
        if (moving_y >= 150 || moving_y <= 0) {
            move_direction *= -1;
        }
        
        // RENDER: Draw to LCD
        LCD_Fill_Buffer(0);
        
        // Title
        LCD_printString("GAME 2", 60, 10, 1, 3);
        
        // Simple animated object (moving box, vertical)
        LCD_printString("[+]", 100, 60 + moving_y, 1, 3);
        
        // Display counter
        char counter[32];
        sprintf(counter, "Frame: %lu", animation_counter);
        LCD_printString(counter, 50, 150, 1, 2);
        
        // Show frame rate
        LCD_printString("Slower Demo", 20, 180, 1, 1);
        LCD_printString("20 FPS", 20, 195, 1, 1);
        
        // Instructions
        LCD_printString("Press BT3 to", 40, 220, 1, 1);
        LCD_printString("Return to Menu", 40, 235, 1, 1);
        
        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME2_FRAME_TIME_MS) {
            HAL_Delay(GAME2_FRAME_TIME_MS - frame_time);
        }
    }
    
    return exit_state;  // Tell main where to go next
}
