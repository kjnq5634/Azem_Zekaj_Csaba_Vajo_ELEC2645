#pragma once
#include <stdint.h>
#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file PWM.h
 * @brief General-purpose PWM library for STM32L4 (LEDs, servos, etc.)
 * 
 * This library uses a struct-based configuration approach similar to the Buzzer
 * and Joystick libraries, allowing flexible timer and channel selection.
 * 
 * Key difference from buzzer: frequency is set first, then duty cycle is adjusted
 * without changing frequency. Ideal for LED brightness, servo control, etc.
 * 
 * Example usage:
 * @code
 * // Configure PWM on TIM4 Channel 1
 * PWM_cfg_t pwm_cfg = {
 *     .htim = &htim4,
 *     .channel = TIM_CHANNEL_1,
 *     .tick_freq_hz = 1000000,  // 1MHz timer clock after prescaler
 *     .min_freq_hz = 10,
 *     .max_freq_hz = 50000,
 *     .setup_done = 0
 * };
 * 
 * // In main:
 * PWM_Init(&pwm_cfg);
 * PWM_SetFreq(&pwm_cfg, 1000);   // 1kHz for LED
 * PWM_SetDuty(&pwm_cfg, 50);     // 50% brightness
 * PWM_SetDuty(&pwm_cfg, 100);    // Full brightness (frequency unchanged)
 * @endcode
 */

/**
 * @struct PWM_cfg_t
 * @brief Configuration for a PWM instance
 * 
 * @details Holds all configuration that defines which timer and channel
 * the PWM is connected to, along with frequency limits.
 */
typedef struct {
    TIM_HandleTypeDef* htim;    ///< Pointer to timer handle (e.g., &htim4)
    uint32_t channel;           ///< Timer channel (e.g., TIM_CHANNEL_1)
    uint32_t tick_freq_hz;      ///< Timer tick frequency after prescaler (Hz)
    uint32_t min_freq_hz;       ///< Minimum frequency limit (Hz)
    uint32_t max_freq_hz;       ///< Maximum frequency limit (Hz)
    uint8_t setup_done;         ///< Internal flag: 1 if initialised, 0 otherwise
    uint8_t pwm_started;        ///< Internal flag: 1 if PWM is running, 0 otherwise
    uint8_t last_duty;          ///< Last set duty cycle for frequency changes
} PWM_cfg_t;

/**
 * @brief Initialise PWM timer
 * 
 * @param cfg Pointer to PWM configuration struct
 * 
 * @details Prepares the timer for PWM operation. PWM output remains OFF
 * until PWM_SetDuty() is called with a non-zero value.
 * 
 * @note Must be called once before using PWM_SetFreq() or PWM_SetDuty()
 */
void PWM_Init(PWM_cfg_t* cfg);

/**
 * @brief Set PWM frequency
 *
 * Changes the PWM output frequency while preserving the last duty cycle.
 * If PWM is running, duty cycle is automatically reapplied at new frequency.
 *
 * @param cfg Pointer to PWM configuration struct
 * @param freq_hz Desired frequency in Hz (clamped to min/max range)
 * 
 * @note Frequency changes don't affect running PWM - duty is preserved
 */
void PWM_SetFreq(PWM_cfg_t* cfg, uint32_t freq_hz);

/**
 * @brief Set PWM duty cycle (brightness/intensity control)
 *
 * @param cfg Pointer to PWM configuration struct
 * @param duty_percent Duty cycle 0..100 (0 => off, 100 => full on)
 * 
 * @note 0% duty stops PWM completely. PWM starts on first non-zero call.
 */
void PWM_SetDuty(PWM_cfg_t* cfg, uint8_t duty_percent);

/**
 * @brief Convenience function: set frequency and duty in one call
 * 
 * @param cfg Pointer to PWM configuration struct
 * @param freq_hz Desired frequency in Hz
 * @param duty_percent Duty cycle 0..100
 */
void PWM_Set(PWM_cfg_t* cfg, uint32_t freq_hz, uint8_t duty_percent);

/**
 * @brief Set PWM timing using raw timer counts (advanced control)
 * 
 * For advanced users who want precise control over ARR (period) and CCR (pulse width)
 * by directly specifying the on and off times in timer ticks.
 * 
 * The PWM frequency = tick_freq_hz / (on_ticks + off_ticks)
 * The duty cycle = on_ticks / (on_ticks + off_ticks)
 * 
 * @param cfg Pointer to PWM configuration struct
 * @param on_ticks Number of ticks the output is HIGH (pulse width)
 * @param off_ticks Number of ticks the output is LOW (dead time)
 * 
 * @note Both on_ticks and off_ticks must be >= 1 for a valid PWM period.
 * If on_ticks == 0, PWM is stopped (same as PWM_Off).
 * 
 * Example: For 1kHz PWM at 25% duty with 1MHz timer tick:
 *   - Total period needed: 1,000,000 / 1000 = 1000 ticks
 *   - on_ticks = 250 (25% of 1000)
 *   - off_ticks = 750 (75% of 1000)
 */
void PWM_SetTicks(PWM_cfg_t* cfg, uint32_t on_ticks, uint32_t off_ticks);

/**
 * @brief Stop PWM (output disabled)
 * 
 * @param cfg Pointer to PWM configuration struct
 */
void PWM_Off(PWM_cfg_t* cfg);

/**
 * @brief Check if PWM is currently running
 * 
 * @param cfg Pointer to PWM configuration struct
 * @return 1 if PWM is active, 0 if stopped
 */
uint8_t PWM_IsRunning(PWM_cfg_t* cfg);

#ifdef __cplusplus
}
#endif