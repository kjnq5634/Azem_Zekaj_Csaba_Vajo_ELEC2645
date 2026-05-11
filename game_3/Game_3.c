#include "Game_3.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern PWM_cfg_t pwm_cfg;      // LED PWM control
extern Buzzer_cfg_t buzzer_cfg; // Buzzer control

/**
 * @brief Game 3 Implementation - Student can modify
 * 
 * EXAMPLE: Shows how to use both PWM LED and Buzzer together
 * This is a placeholder with a diagonal bouncing animation.
 * Replace this with your actual game logic!
 */

// Game state - customize for your game
static uint32_t animation_counter = 0;
static int16_t moving_x = 0;
static int16_t moving_y = 0;
static int8_t dx = 2;
static int8_t dy = 2;

// Frame rate for this game (in milliseconds) - fastest game
#define GAME3_FRAME_TIME_MS 16  // ~60 FPS (faster than others!)

MenuState Game3_Run(void) {
    // Initialize game state
    animation_counter = 0;
    moving_x = 50;
    moving_y = 50;
    dx = 2;
    dy = 2;
    
    // Play a brief startup sound
    buzzer_tone(&buzzer_cfg, 1500, 30);  // 1.5kHz at 30% volume
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
            PWM_SetDuty(&pwm_cfg, 50);  // Reset LED
            exit_state = MENU_STATE_HOME;
            break;  // Exit game loop
        }
        
        // UPDATE: Game logic
        animation_counter++;
        
        // Simple animation: move object diagonally
        moving_x += dx;
        moving_y += dy;
        if (moving_x >= 200 || moving_x <= 0) {
            dx *= -1;
        }
        if (moving_y >= 200 || moving_y <= 0) {
            dy *= -1;
        }
        
        // Example: Vary LED brightness based on distance from origin
        uint8_t brightness = 30 + ((moving_x + moving_y) * 40) / 400;
        if (brightness > 100) brightness = 100;
        PWM_SetDuty(&pwm_cfg, brightness);
        
        // RENDER: Draw to LCD
        LCD_Fill_Buffer(0);
        
        // Title
        LCD_printString("GAME 3", 60, 10, 1, 3);
        
        // Simple animated object (moving box, diagonal)
        LCD_printString("[o]", 20 + moving_x, 50 + moving_y, 1, 3);
        
        // Display counter
        char counter[32];
        sprintf(counter, "Frame: %lu", animation_counter);
        LCD_printString(counter, 50, 140, 1, 2);
        
        // Show frame rate
        LCD_printString("Fast Demo", 20, 180, 1, 1);
        LCD_printString("60 FPS", 20, 195, 1, 1);
        
        // Instructions
        LCD_printString("Press BT3 to", 40, 220, 1, 1);
        LCD_printString("Return to Menu", 40, 235, 1, 1);
        
        LCD_Refresh(&cfg0);
        
        // Frame timing - wait for remainder of frame time
        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME3_FRAME_TIME_MS) {
            HAL_Delay(GAME3_FRAME_TIME_MS - frame_time);
        }
    }
    
    return exit_state;  // Tell main where to go next
}
