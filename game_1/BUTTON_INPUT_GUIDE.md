# How to Implement Your Game

## Overview

The menu system uses a **simple, centralized game loop** in `main.c` that follows the classic game development pattern:

```
while(1) {
    Read Input       → Input_Read()
        ↓
    Update Logic     → Menu_Update() or GameX_Update()
        ↓
    Render Screen    → Menu_Render() or GameX_Render()
}
```

Your game simply implements three functions: `Init()`, `Update()`, and `Render()`.

## The Three Functions You Need to Implement

### 1. `Game1_Init()` - Initialize Your Game

Called once when your game is first selected from the menu.

```c
static int score = 0;
static int player_x = 120;

void Game1_Init(void) {
    score = 0;
    player_x = 120;
    // Initialize any game resources, sprites, state, etc.
}
```

### 2. `Game1_Update()` - Update Game Logic

Called **once per frame** (~30 FPS) while your game is running. Handle all game logic here:
- Check button input
- Move objects
- Check collisions
- Update game state

```c
void Game1_Update(void) {
    extern MenuSystem menu;        // Access the menu state
    extern InputState current_input;  // Access button state
    
    // Check if button was pressed to return to menu
    if (current_input.btn3_pressed) {
        Menu_BackToHome(&menu);
        return;
    }
    
    // Your game logic here
    score++;
    
    if (score > 1000) {
        score = 0;  // Reset score
    }
}
```

### 3. `Game1_Render()` - Draw Your Game

Called **once per frame** after `Update()`. Draw everything to the LCD.

```c
void Game1_Render(void) {
    extern ST7789V2_cfg_t cfg0;  // LCD configuration
    
    LCD_Fill_Buffer(0);  // Clear screen
    
    // Draw game title
    LCD_printString("GAME 1", 60, 10, 1, 3);
    
    // Draw score
    char score_str[32];
    sprintf(score_str, "Score: %d", score);
    LCD_printString(score_str, 50, 100, 1, 2);
    
    // Draw instructions
    LCD_printString("Press BT3", 50, 240, 1, 1);
    LCD_printString("to return", 50, 250, 1, 1);
    
    LCD_Refresh(&cfg0);  // Update display
}
```

## Accessing Global State

Your game functions have access to several global variables:

### Button Input

```c
extern InputState current_input;

void Game1_Update(void) {
    // Check if BT3 was pressed this frame
    if (current_input.btn3_pressed) {
        // Button 3 pressed - do something
    }
    
    // Check if BT2 was pressed this frame  
    if (current_input.btn2_pressed) {
        // Button 2 pressed - do something
    }
}
```

### Menu State (to return to menu)

```c
extern MenuSystem menu;

void Game1_Update(void) {
    if (some_game_over_condition) {
        Menu_BackToHome(&menu);  // Return to menu
        return;
    }
}
```

### LCD for drawing

```c
extern ST7789V2_cfg_t cfg0;

void Game1_Render(void) {
    LCD_Fill_Buffer(0);
    LCD_printString("Hello", 50, 50, 1, 2);
    LCD_Refresh(&cfg0);
}
```

### Timing

The main loop calls your Update/Render about **30 times per second** (FRAME_TIME_MS = 30ms). You can use this in your game:

```c
static uint32_t frame_counter = 0;

void Game1_Update(void) {
    frame_counter++;
    
    // Do something every 10 frames (approx 3 times per second)
    if (frame_counter % 10 == 0) {
        // ...
    }
}
```

## Example Game: Simple Bouncing Ball

Here's a complete example of a simple game:

```c
// ===== in game_1/Game_1.c =====

#include "Game_1.h"
#include "LCD.h"
#include "InputHandler.h"
#include "Menu.h"

// Game state
static int ball_x = 120;
static int ball_y = 160;
static int vel_x = 3;
static int vel_y = 2;
static int score = 0;

void Game1_Init(void) {
    ball_x = 120;
    ball_y = 160;
    vel_x = 3;
    vel_y = 2;
    score = 0;
}

void Game1_Update(void) {
    extern MenuSystem menu;
    extern InputState current_input;
    extern ST7789V2_cfg_t cfg0;
    
    // Return to menu on button press
    if (current_input.btn3_pressed) {
        Menu_BackToHome(&menu);
        return;
    }
    
    // Move ball
    ball_x += vel_x;
    ball_y += vel_y;
    
    // Bounce off walls
    if (ball_x <= 0 || ball_x >= 230) {
        vel_x = -vel_x;
    }
    if (ball_y <= 0 || ball_y >= 230) {
        vel_y = -vel_y;
    }
    
    // Increment score
    score++;
}

void Game1_Render(void) {
    extern ST7789V2_cfg_t cfg0;
    
    LCD_Fill_Buffer(0);
    
    // Draw title
    LCD_printString("GAME 1", 60, 10, 1, 2);
    
    // Draw ball as a simple character
    char ball_char = 'O';
    LCD_printString(&ball_char, ball_x, ball_y, 1, 2);
    
    // Draw score
    char score_str[32];
    sprintf(score_str, "Score: %d", score);
    LCD_printString(score_str, 50, 200, 1, 1);
    
    LCD_Refresh(&cfg0);
}
```

## How to Return to Menu

In your `Game_Update()` function:

```c
void Game1_Update(void) {
    extern MenuSystem menu;
    extern InputState current_input;
    
    // Player presses button to return to menu
    if (current_input.btn3_pressed) {
        Menu_BackToHome(&menu);
        return;
    }
    
    // Your normal game logic...
}
```

Or if the game ends (player dies, wins, etc):

```c
void Game1_Update(void) {
    extern MenuSystem menu;
    
    // Check game over condition
    if (lives == 0) {
        Menu_BackToHome(&menu);
        return;
    }
    
    // Your game logic...
}
```

## Key Points

- ✅ Keep `Init()`, `Update()`, and `Render()` separate
- ✅ Check `current_input.btn3_pressed` in your Update function
- ✅ Call `Menu_BackToHome(&menu)` to return to menu
- ✅ Use `LCD_printString()` and `LCD_Refresh()` to draw
- ✅ Use static variables to maintain state between frames
- ✅ Remember the main loop runs at ~30 FPS (FRAME_TIME_MS = 30ms)

## Summary

The system is simple:
1. Call your three functions each frame from the main loop
2. Access input from `current_input`
3. Draw to LCD in your Render function
4. Return to menu by calling `Menu_BackToHome()`

No callbacks, no complex state machines - just straightforward game logic!

