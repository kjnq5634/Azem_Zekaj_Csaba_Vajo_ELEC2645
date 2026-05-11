# ELEC2645 Collaborative Game Menu

Our games combined into one Menu for the Group Project, demonstrating how we can implement our own games within a shared menu system and game loop architecture.

## Overview

This project provides:
- **Centralized Menu System**: Navigate between our 2 independent games
- **Simple Game Loop**: INPUT → UPDATE → RENDER pattern
- **Shared Resources**: LCD display, joystick input, buzzer, PWM LED
- **Student Friendly**: Each of our games are in their own folder
- **No Merge Conflicts**: Shared code stays in `shared/`, each game in its own folder, extra files are within folders named after us

## Project Structure

```
MenuTest/
├── Core/              # STM32 auto-generated files
├── Drivers/           # STM32 HAL drivers
├── shared/            # Shared menu system & input handling
│   ├── Menu.h/c
│   └── InputHandler.h/c
├── game_1/            # Azem's game
│   └── Game_1.c
├── game_2/            # cSABA's game
│   └── Game_2.c
├── Joystick/          # Hardware drivers
├── PWM/
├── Buzzer/
└── CMakeLists.txt
```

## Quick Start

## Architecture

### Main Game Loop (main.c)

```c
while(1) {
    Input_Read();              // Read button and joystick
    
    switch(current_state) {    // UPDATE
        case MENU: Menu_Update(); break;
        case GAME_1: Game1_Update(); break;
        case GAME_2: Game2_Update(); break;
        case GAME_3: Game3_Update(); break;
    }
    
    switch(current_state) {    // RENDER
        case MENU: Menu_Render(); break;
        case GAME_1: Game1_Render(); break;
        case GAME_2: Game2_Render(); break;
        case GAME_3: Game3_Render(); break;
    }
}
```

### Each Game Implements Three Functions

```c
void GameX_Init(void);      // Called once when game is selected from menu
void GameX_Update(void);    // Called every frame (~30 FPS)
void GameX_Render(void);    // Called every frame (after Update)
```

## Controls

- **Joystick**: Custom game use, see game startup screens for control
- **BT2 Button**: Custom game use, see game startup screens for controls
- **BT3 Button**: Custom game use, see game startup screens for control

## Hardware Features

- **STM32L476 Microcontroller**
- **ST7789V2 LCD Display** (240×320)
- **Joystick Input** with 8-directional output
- **PWM LED** for visual effects
- **Buzzer** for sound effects
- **Timers**: TIM6 (100Hz) and TIM7 (1Hz) available for game timing

See driver folders (Joystick/, PWM/, Buzzer/) for API documentation.
