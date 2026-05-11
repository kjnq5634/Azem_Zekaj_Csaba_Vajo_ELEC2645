#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdint.h>

// ========================================
// INPUT SYSTEM - Simple State
// ========================================

/**
 * @brief Simple input state structure
 * 
 * Store the current input state. Main loop checks this to determine
 * what happened this frame.
 */
typedef struct {
    uint8_t btn2_pressed;  // 1 if BT2 was pressed this frame, 0 otherwise
    uint8_t btn3_pressed;  // 1 if BT3 was pressed this frame, 0 otherwise
} InputState;

// Global input state (read by menu/games in their Update functions)
extern InputState current_input;

/**
 * @brief Initialize button input system
 * Must be called in main() after GPIO initialization
 */
void Input_Init(void);

/**
 * @brief Read current input state
 * Called once per frame by main loop before Update
 */
void Input_Read(void);

#endif // INPUT_HANDLER_H
