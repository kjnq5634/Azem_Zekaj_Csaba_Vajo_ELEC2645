#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "GameSprites.h"
#include "Background.h"
#include "DuckEngine.h"
#include "Joystick.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdint.h>

extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;

static DuckEngine_t duck_engine;


static uint32_t game_start_time = 0;
static uint8_t night_mode = 0;
static uint8_t last_night_mode = 255;
static uint8_t game_over= 0;

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


static void update_duckhunt(Joystick_t* joy, uint8_t fire_pressed, uint8_t reload_pressed);
static void render_duckhunt(void);
static void Draw_Ammo_UI(uint8_t);
static void Draw_Game_Over_Screen(void);

// Frame rate for this game (in milliseconds) - runs slower than Game 1
#define GAME2_FRAME_TIME_MS 50  // ~20 FPS (different from Game 1!)

MenuState Game2_Run(void) {
    MenuState exit_state = MENU_STATE_HOME;
    game_over = 0;
    night_mode = 0;
    last_night_mode = 255;
    game_start_time = HAL_GetTick();
 
    DuckEngine_Init(&duck_engine); //reset score, lives, ammo, crosshair and then spawn ducks
    LCD_Fill_Buffer(0); 
    LCD_Refresh(&cfg0);

    //first title screen
    LCD_printString("DUCK HUNT", 35, 40, 15, 3);
    LCD_Refresh(&cfg0);
    HAL_Delay(1000);

    //instruction screen to explain controls before game
    LCD_Fill_Buffer(0);
    LCD_printString("Move Joystick", 30, 35, 15, 2);
    LCD_printString("to AIM", 70, 65, 15, 2);
    LCD_printString("Press BTN2", 45, 105, 15, 2);
    LCD_printString("to SHOOT", 50, 135, 15, 2);
    LCD_printString("BTN3 on Joystick", 35, 155, 15, 2);
    LCD_printString("to Reload", 50, 175, 15, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(1800);

    //final screen before game
    LCD_Fill_Buffer(0);
    LCD_printString("Shoot Ducks!", 45, 50, 15, 2);
    LCD_printString("Dont miss!", 55, 90, 11, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(1200);

    printf("Duck Hunt initialized\n");
    game_start_time = HAL_GetTick();
    uint32_t last_tick = HAL_GetTick();
    
    while (!game_over)
    {
        uint32_t now = HAL_GetTick();

        //limit update/render to 60FPS
        if ((now - last_tick) < FRAME_TIME_MS) {
            continue;
        }
        last_tick = now;

        Joystick_Read(&joystick_cfg, &joystick_data); //read joystick pos for crosshair movement

        Input_Read(); //read button inputs

        uint8_t fire_pressed = current_input.btn2_pressed; //breadboard button that shoots
        uint8_t reload_pressed = current_input.btn3_pressed; //joystick button reloads

        //update gameplay logic then draws new frame
        update_duckhunt(&joystick_data, fire_pressed, reload_pressed);
        render_duckhunt();
    }

    while (1)
    {
        //game over screen
        LCD_Fill_Buffer(11);
        LCD_printString("GAME OVER", 40, 40, 15, 3);

        char score_str[32];
        sprintf(score_str, "Score: %d", DuckEngine_GetScore(&duck_engine));
        LCD_printString(score_str, 55, 95, 15, 2);

        LCD_printString("Press RESET", 50, 140, 15, 2);

        LCD_Refresh(&cfg0);
        HAL_Delay(200);
    }
    buzzer_off(&buzzer_cfg);
    return exit_state;  //tell main where to go next
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
