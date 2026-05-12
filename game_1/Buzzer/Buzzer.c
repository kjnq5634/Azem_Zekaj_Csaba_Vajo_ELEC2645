#include "Buzzer.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>

/**
 * @file Buzzer.c
 * @brief Implementation of flexible PWM buzzer driver
 * 
 * This implementation supports any timer and channel combination,
 * following the same configuration struct pattern as the Joystick
 * and LCD libraries for consistency.
 */

static inline uint32_t clamp_u32(uint32_t x, uint32_t lo, uint32_t hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void buzzer_init(Buzzer_cfg_t* cfg)
{
    if (!cfg->setup_done) {
        // Timer should be initialized by CubeMX (MX_TIMx_Init)
        // This just marks the buzzer as ready to use
        cfg->pwm_started = 0;
        cfg->setup_done = 1;
    }
}

uint8_t buzzer_is_running(Buzzer_cfg_t* cfg)
{
    return cfg->pwm_started ? 1u : 0u;
}

void buzzer_off(Buzzer_cfg_t* cfg)
{
    if (cfg->pwm_started) {
        // Make sure duty is zero first, then stop PWM fully
        __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, 0);
        HAL_TIM_PWM_Stop(cfg->htim, cfg->channel);
        cfg->pwm_started = 0;
    }
}

void buzzer_tone(Buzzer_cfg_t* cfg, uint32_t freq_hz, uint8_t volume_percent)
{
    // Auto-initialize if not done yet
    if (!cfg->setup_done) {
        buzzer_init(cfg);
    }

    // volume 0 => fully off
    if (volume_percent == 0 || freq_hz == 0) {
        buzzer_off(cfg);
        return;
    }

    // Clamp frequency to configured range
    freq_hz = clamp_u32(freq_hz, cfg->min_freq_hz, cfg->max_freq_hz);

    // Start PWM ONLY when first used -> buzzer is completely off until called
    if (!cfg->pwm_started) {
        // Start PWM output
        HAL_TIM_PWM_Start(cfg->htim, cfg->channel);
        cfg->pwm_started = 1;
    }

    // ============ FREQUENCY CONTROL ============
    // The timer generates a PWM signal at frequency f_pwm = timer_tick_freq / (ARR + 1)
    // Where:
    //   - timer_tick_freq = timer input clock / prescaler (e.g., 1MHz from prescaler=79)
    //   - ARR = Auto-Reload Register value (timer period)
    // 
    // To play a desired frequency, we calculate: ARR = (timer_tick_freq / freq_hz) - 1
    // 
    // Example: To play 440Hz with 1MHz timer tick:
    //   ARR = (1,000,000 / 440) - 1 = 2272
    //   This gives a period of 2273 ticks = 2.273ms (1/440Hz = 2.27ms) ✓
    
    uint32_t arr = (cfg->tick_freq_hz / freq_hz) - 1u;

    // ARR must be >= 1 for a meaningful PWM period
    arr = clamp_u32(arr, 1u, 0xFFFFFFFFu);

    // Update ARR and reset the counter for a clean phase start
    __HAL_TIM_SET_AUTORELOAD(cfg->htim, arr);
    __HAL_TIM_SET_COUNTER(cfg->htim, 0);

    // Force registers reload immediately (safe with ARPE enabled)
    HAL_TIM_GenerateEvent(cfg->htim, TIM_EVENTSOURCE_UPDATE);

    // ============ DUTY CYCLE / VOLUME CONTROL ============
    // Duty cycle controls the "volume" - how loud the tone sounds
    // Duty cycle = CCR / (ARR + 1), where CCR is the Compare register value
    // 
    // This implementation uses up to 50% duty cycle for a cleaner square-wave tone.
    // This avoids extreme values and produces a perceptually "normal" buzzer sound.
    // 
    // Volume mapping: 0% volume => 0% duty, 100% volume => 50% duty
    // 
    // Example: If ARR=2272 (2273 total counts):
    //   - half_period = 2273 / 2 = 1136 counts
    //   - At 50% volume: CCR = 1136 * 50 / 100 = 568 counts
    //   - Duty = 568 / 2273 ≈ 25% of period (half of our 50% max)
    
    volume_percent = (uint8_t)clamp_u32(volume_percent, 0u, 100u);

    uint32_t half_period = (arr + 1u) / 2u;
    uint32_t ccr = (half_period * volume_percent) / 100u;

    __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, ccr);
}

void buzzer_note(Buzzer_cfg_t* cfg, Buzzer_Note_t note, uint8_t volume_percent)
{
    // Simply play the note frequency (note enum contains the Hz value)
    buzzer_tone(cfg, (uint32_t)note, volume_percent);
}