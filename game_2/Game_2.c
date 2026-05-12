#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdint.h>

static uint32_t game_start_time = 0;
uint8_t night_mode = 0;
static uint8_t last_night_mode = 255;

#define NIGHT_START_MS 5000 //when night mode starts in game
#define NIGHT_DURATION_MS 5000 //how long night mode lasts

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

void update_duckhunt(Joystick_t* joy, uint8_t fire_pressed, uint8_t reload_pressed);
void render_duckhunt(void);

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

void render_duckhunt(void)
{
    static uint8_t first_frame = 1;
    char info_str[32];
    //only redraw background on first frame or when game switches from night to day and vice versa
    //to avoid any flickering for user
    if (first_frame || night_mode != last_night_mode) {
        if (night_mode) {
            LCD_Fill_Buffer(1); //night sky background colour
            Background_Draw_Night(); //draws night scenery
        } else {
            LCD_Fill_Buffer(7); //daytime sky background colour
            Background_Draw(); //draw day scenery
        }
        //draw sniper as a static foreground object once the background is drawn
        Sniper_Draw(165, 179);
        first_frame = 0;
        last_night_mode = night_mode;
    }
    //draw moving objects like ducks, crosshair, gun, hitmarker
    DuckEngine_Draw(&duck_engine, night_mode ? 1 : 7, night_mode);

    //clear and redraw score/lives each frame
    LCD_Draw_Rect(0, 0, 240, 22, night_mode ? 1 : 7, 1);
    sprintf(info_str, "Lives:%d", DuckEngine_GetLives(&duck_engine));
    LCD_printString(info_str, 5, 5, 15, 2);
    sprintf(info_str, "Score:%d", DuckEngine_GetScore(&duck_engine));
    LCD_printString(info_str, 135, 5, 15, 2);

    //draw ammo display at bottom left
    Draw_Ammo_UI(DuckEngine_GetAmmo(&duck_engine));
    LCD_Refresh(&cfg0);
}

static void Draw_Ammo_UI(uint8_t ammo)
{
    const uint16_t start_x = 8;
    const uint16_t start_y = 202;
    const uint16_t bullet_w = 7;
    const uint16_t bullet_h = 28;
    const uint16_t gap = 4;

    for (uint8_t i = 0; i < 6; i++) { //draw six bullet icons at bottom left of screen
        uint16_t x = start_x + i * (bullet_w + gap);
        uint16_t y = start_y;

        //empty bullet black
        LCD_Draw_Rect(x, y, bullet_w, bullet_h, 0, 1);

        //white bullet border
        LCD_Draw_Rect(x, y, bullet_w, bullet_h, 15, 0);

        //bright pink fill if bullet is loaded
        if (i < ammo) {
            LCD_Draw_Rect(x + 2, y + 2, bullet_w - 4, bullet_h - 4, 12, 1);
        }
    }
}

//Update and render functions:
//update function
void update_duckhunt(Joystick_t* joy, uint8_t fire_pressed, uint8_t reload_pressed)
{
    uint32_t elapsed = HAL_GetTick() - game_start_time; //workout how long game has been running

    //enables night mode for fixed amount of time
    if (elapsed >= NIGHT_START_MS && elapsed < NIGHT_START_MS + NIGHT_DURATION_MS) {
        night_mode = 1;
    } else {
        night_mode = 0;
    }

    //slow ducks down during night mode, otherwise use normal speed multiplier, the short hand for an if statement is used quite a bit when convenient for me
    //in actuality the 0.5 rounds down to 0 but I like to keep it 0.5 to indicate here as well that its half speed
    DuckEngine_SetSpeedMultiplier(&duck_engine, night_mode ? 0.5 : 1);

    uint8_t lives = DuckEngine_Update(&duck_engine, joy, fire_pressed, reload_pressed);

    if (lives == 0) { //end game when all lives are lost
        buzzer_off(&buzzer_cfg);
        game_over = 1;
    }
}

static void Draw_Game_Over_Screen(void) {
    char store_str[32];
    LCD_Fill_Buffer(11);
    LCD_printString("GAME OVER", 40,40,15,3);
    
}
