#pragma once
#include <stdint.h>
#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file Buzzer.h
 * @brief Flexible PWM-based buzzer library for STM32L4
 * 
 * This library uses a struct-based configuration approach similar to the Joystick
 * and LCD libraries, allowing flexible timer and channel selection.
 * 
 * Features:
 * - Play tones at arbitrary frequencies
 * - Play musical notes (C4-C7) with symbolic names, including sharps/flats
 * - Volume control (0-100%)
 * 
 * Example usage:
 * @code
 * // Configure buzzer on TIM2 Channel 3
 * Buzzer_cfg_t buzzer_cfg = {
 *     .htim = &htim2,
 *     .channel = TIM_CHANNEL_3,
 *     .tick_freq_hz = 1000000,  // 1MHz timer clock after prescaler
 *     .min_freq_hz = 20,
 *     .max_freq_hz = 20000,
 *     .setup_done = 0
 * };
 * 
 * // In main loop:
 * buzzer_tone(&buzzer_cfg, 440, 50);       // Play 440Hz at 50% volume
 * buzzer_note(&buzzer_cfg, NOTE_C4, 50);   // Play musical note C4 at 50% volume
 * buzzer_note(&buzzer_cfg, NOTE_CS5, 50);  // Play C# in octave 5
 * buzzer_off(&buzzer_cfg);                 // Turn off
 * @endcode
 */

/**
 * @enum Buzzer_Note_t
 * @brief Musical notes C4 to C8 with sharps/flats
 * 
 * Standard musical notation: C, C#/Db, D, D#/Eb, E, F, F#/Gb, G, G#/Ab, A, A#/Bb, B
 * Octaves 4-8 (C4 to C8)
 */
typedef enum {
    // Octave 4
    NOTE_C4  = 262,   ///< C4: 262 Hz
    NOTE_CS4 = 277,   ///< C#4/Db4: 277 Hz
    NOTE_D4  = 294,   ///< D4: 294 Hz
    NOTE_DS4 = 311,   ///< D#4/Eb4: 311 Hz
    NOTE_E4  = 330,   ///< E4: 330 Hz
    NOTE_F4  = 349,   ///< F4: 349 Hz
    NOTE_FS4 = 370,   ///< F#4/Gb4: 370 Hz
    NOTE_G4  = 392,   ///< G4: 392 Hz
    NOTE_GS4 = 415,   ///< G#4/Ab4: 415 Hz
    NOTE_A4  = 440,   ///< A4: 440 Hz (standard tuning reference)
    NOTE_AS4 = 466,   ///< A#4/Bb4: 466 Hz
    NOTE_B4  = 494,   ///< B4: 494 Hz
    
    // Octave 5
    NOTE_C5  = 523,   ///< C5: 523 Hz
    NOTE_CS5 = 554,   ///< C#5/Db5: 554 Hz
    NOTE_D5  = 587,   ///< D5: 587 Hz
    NOTE_DS5 = 622,   ///< D#5/Eb5: 622 Hz
    NOTE_E5  = 659,   ///< E5: 659 Hz
    NOTE_F5  = 698,   ///< F5: 698 Hz
    NOTE_FS5 = 740,   ///< F#5/Gb5: 740 Hz
    NOTE_G5  = 784,   ///< G5: 784 Hz
    NOTE_GS5 = 831,   ///< G#5/Ab5: 831 Hz
    NOTE_A5  = 880,   ///< A5: 880 Hz
    NOTE_AS5 = 932,   ///< A#5/Bb5: 932 Hz
    NOTE_B5  = 988,   ///< B5: 988 Hz
    
    // Octave 6
    NOTE_C6  = 1047,  ///< C6: 1047 Hz
    NOTE_CS6 = 1109,  ///< C#6/Db6: 1109 Hz
    NOTE_D6  = 1175,  ///< D6: 1175 Hz
    NOTE_DS6 = 1245,  ///< D#6/Eb6: 1245 Hz
    NOTE_E6  = 1319,  ///< E6: 1319 Hz
    NOTE_F6  = 1397,  ///< F6: 1397 Hz
    NOTE_FS6 = 1480,  ///< F#6/Gb6: 1480 Hz
    NOTE_G6  = 1568,  ///< G6: 1568 Hz
    NOTE_GS6 = 1661,  ///< G#6/Ab6: 1661 Hz
    NOTE_A6  = 1760,  ///< A6: 1760 Hz
    NOTE_AS6 = 1865,  ///< A#6/Bb6: 1865 Hz
    NOTE_B6  = 1976,  ///< B6: 1976 Hz
    
    // Octave 7
    NOTE_C7  = 2093,  ///< C7: 2093 Hz
    NOTE_CS7 = 2217,  ///< C#7/Db7: 2217 Hz
    NOTE_D7  = 2349,  ///< D7: 2349 Hz
    NOTE_DS7 = 2489,  ///< D#7/Eb7: 2489 Hz
    NOTE_E7  = 2637,  ///< E7: 2637 Hz
    NOTE_F7  = 2794,  ///< F7: 2794 Hz
    NOTE_FS7 = 2960,  ///< F#7/Gb7: 2960 Hz
    NOTE_G7  = 3136,  ///< G7: 3136 Hz
    NOTE_GS7 = 3322,  ///< G#7/Ab7: 3322 Hz
    NOTE_A7  = 3520,  ///< A7: 3520 Hz
    NOTE_AS7 = 3729,  ///< A#7/Bb7: 3729 Hz
    NOTE_B7  = 3951,  ///< B7: 3951 Hz
    
    // Octave 8
    NOTE_C8  = 4186   ///< C8: 4186 Hz
} Buzzer_Note_t;

/**
 * @struct Buzzer_cfg_t
 * @brief Configuration for a buzzer instance
 * 
 * @details Holds all configuration that defines which timer and channel
 * the buzzer is connected to, along with frequency limits.
 */
typedef struct {
    TIM_HandleTypeDef* htim;    ///< Pointer to timer handle (e.g., &htim2)
    uint32_t channel;           ///< Timer channel (e.g., TIM_CHANNEL_3)
    uint32_t tick_freq_hz;      ///< Timer tick frequency after prescaler (Hz)
    uint32_t min_freq_hz;       ///< Minimum audible frequency (Hz)
    uint32_t max_freq_hz;       ///< Maximum audible frequency (Hz)
    uint8_t setup_done;         ///< Internal flag: 1 if initialized, 0 otherwise
    uint8_t pwm_started;        ///< Internal flag: 1 if PWM is running, 0 otherwise
} Buzzer_cfg_t;

/**
 * @brief Initialize buzzer timer
 * 
 * @param cfg Pointer to buzzer configuration struct
 * 
 * @details Prepares the timer for PWM operation. The buzzer remains
 * OFF until buzzer_tone() is called.
 * 
 * @note Must be called once before using buzzer_tone()
 */
void buzzer_init(Buzzer_cfg_t* cfg);

/**
 * @brief Play a tone on the buzzer
 *
 * The buzzer is completely OFF until the first time this function is called.
 *
 * @param cfg Pointer to buzzer configuration struct
 * @param freq_hz Tone frequency in Hz (e.g., 440)
 * @param volume_percent 0..100 (0 => off)
 */
void buzzer_tone(Buzzer_cfg_t* cfg, uint32_t freq_hz, uint8_t volume_percent);

/**
 * @brief Play a musical note on the buzzer
 *
 * Convenience function for playing pre-defined musical notes (C4-C5).
 * Internally calls buzzer_tone() with the note's frequency.
 *
 * @param cfg Pointer to buzzer configuration struct
 * @param note Musical note from Buzzer_Note_t enum (NOTE_C4 to NOTE_C5)
 * @param volume_percent 0..100 (0 => off)
 */
void buzzer_note(Buzzer_cfg_t* cfg, Buzzer_Note_t note, uint8_t volume_percent);

/**
 * @brief Stop the buzzer (PWM fully stopped, output disabled)
 * 
 * @param cfg Pointer to buzzer configuration struct
 */
void buzzer_off(Buzzer_cfg_t* cfg);

/**
 * @brief Check if the buzzer is currently running
 * 
 * @param cfg Pointer to buzzer configuration struct
 * @return 1 if PWM is running, 0 otherwise
 */
uint8_t buzzer_is_running(Buzzer_cfg_t* cfg);

#ifdef __cplusplus
}
#endif