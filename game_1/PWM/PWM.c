#include "PWM.h"
#include "stm32l4xx_hal.h"

/**
 * @file PWM.c
 * @brief Implementation of general-purpose PWM driver
 * 
 * This implementation supports any timer and channel combination,
 * following the same configuration struct pattern as the Buzzer library.
 * 
 * Key features:
 * - Frequency can be changed while preserving duty cycle
 * - Duty cycle updates don't restart PWM
 * - Ideal for LED brightness, servo control, motor speed
 */

static inline uint32_t clamp_u32(uint32_t x, uint32_t lo, uint32_t hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void PWM_Init(PWM_cfg_t* cfg)
{
    if (!cfg->setup_done) {
        // Timer should be initialised by CubeMX (MX_TIMx_Init)
        // This just marks the PWM as ready to use
        cfg->pwm_started = 0;
        cfg->last_duty = 0;
        cfg->setup_done = 1;
    }
}

uint8_t PWM_IsRunning(PWM_cfg_t* cfg)
{
    return cfg->pwm_started ? 1u : 0u;
}

void PWM_Off(PWM_cfg_t* cfg)
{
    if (cfg->pwm_started) {
        // Make sure duty is zero first, then stop PWM fully
        __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, 0);
        HAL_TIM_PWM_Stop(cfg->htim, cfg->channel);
        cfg->pwm_started = 0;
    }
    cfg->last_duty = 0;
}

static void apply_duty_at_current_frequency(PWM_cfg_t* cfg, uint8_t duty_percent)
{
    // Get current ARR (set by PWM_SetFreq)
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(cfg->htim);
    uint32_t top = arr + 1u;

    duty_percent = (uint8_t)clamp_u32(duty_percent, 0u, 100u);
    uint32_t ccr = (top * (uint32_t)duty_percent) / 100u;

    // Clamp CCR to [0..ARR]
    if (ccr > arr) ccr = arr;

    __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, ccr);
}

void PWM_SetFreq(PWM_cfg_t* cfg, uint32_t freq_hz)
{
    // Auto-initialise if not done yet
    if (!cfg->setup_done) {
        PWM_Init(cfg);
    }

    // ============ FREQUENCY CONTROL ============
    // The timer generates a PWM signal at frequency f_pwm = timer_tick_freq / (ARR + 1)
    // To play a desired frequency, we calculate: ARR = (timer_tick_freq / freq_hz) - 1
    
    // Clamp to configured range
    freq_hz = clamp_u32(freq_hz, cfg->min_freq_hz, cfg->max_freq_hz);

    // Calculate ARR for desired frequency
    uint32_t arr = (cfg->tick_freq_hz / freq_hz) - 1u;

    // ARR must fit in timer register (typically 16-bit for TIM4: max 65535)
    arr = clamp_u32(arr, 1u, 65535u);

    // Update ARR and reset counter for clean phase start
    __HAL_TIM_SET_AUTORELOAD(cfg->htim, arr);
    __HAL_TIM_SET_COUNTER(cfg->htim, 0);

    // Force registers reload immediately (safe with ARPE enabled)
    HAL_TIM_GenerateEvent(cfg->htim, TIM_EVENTSOURCE_UPDATE);

    // Preserve duty cycle at new frequency (if PWM is running)
    if (cfg->pwm_started && cfg->last_duty > 0) {
        apply_duty_at_current_frequency(cfg, cfg->last_duty);
    }
}

void PWM_SetDuty(PWM_cfg_t* cfg, uint8_t duty_percent)
{
    // Auto-initialise if not done yet
    if (!cfg->setup_done) {
        PWM_Init(cfg);
    }

    // ============ DUTY CYCLE / BRIGHTNESS CONTROL ============
    // Duty cycle = CCR / (ARR + 1)
    // At 0% duty, PWM stops completely
    // At 100% duty, output stays high (CCR = ARR)
    
    // 0% => fully off
    if (duty_percent == 0) {
        PWM_Off(cfg);
        return;
    }

    // Start PWM ONLY when first used with non-zero duty
    if (!cfg->pwm_started) {
        HAL_TIM_PWM_Start(cfg->htim, cfg->channel);
        cfg->pwm_started = 1;
    }

    // Remember this duty for frequency changes
    cfg->last_duty = duty_percent;
    apply_duty_at_current_frequency(cfg, duty_percent);
}

void PWM_Set(PWM_cfg_t* cfg, uint32_t freq_hz, uint8_t duty_percent)
{
    // If duty is zero, shut down completely
    if (duty_percent == 0 || freq_hz == 0) {
        PWM_Off(cfg);
        return;
    }

    // Set frequency first (so duty is computed against correct ARR)
    PWM_SetFreq(cfg, freq_hz);
    PWM_SetDuty(cfg, duty_percent);
}

void PWM_SetTicks(PWM_cfg_t* cfg, uint32_t on_ticks, uint32_t off_ticks)
{
    // Auto-initialise if not done yet
    if (!cfg->setup_done) {
        PWM_Init(cfg);
    }

    // ============ DIRECT TICK CONTROL ============
    // This provides low-level control by directly setting:
    //   - ARR (Auto-Reload Register) = on_ticks + off_ticks - 1
    //   - CCR (Compare Register) = on_ticks - 1
    // 
    // PWM period = (ARR + 1) ticks = (on_ticks + off_ticks) ticks
    // Pulse width = CCR ticks = on_ticks ticks
    // Frequency = tick_freq_hz / (on_ticks + off_ticks)
    
    // If on_ticks is zero, turn off the PWM completely
    if (on_ticks == 0) {
        PWM_Off(cfg);
        return;
    }

    // Both timing values must be >= 1 for a valid PWM period
    on_ticks = clamp_u32(on_ticks, 1u, 65535u);
    off_ticks = clamp_u32(off_ticks, 1u, 65535u);

    // Total period: on + off
    uint32_t total_ticks = on_ticks + off_ticks;
    
    // ARR must be >= 1 and fit in the 16-bit timer register
    uint32_t arr = total_ticks - 1u;
    arr = clamp_u32(arr, 1u, 65535u);

    // Update ARR (period) and reset counter
    __HAL_TIM_SET_AUTORELOAD(cfg->htim, arr);
    __HAL_TIM_SET_COUNTER(cfg->htim, 0);

    // Start PWM if not already running
    if (!cfg->pwm_started) {
        HAL_TIM_PWM_Start(cfg->htim, cfg->channel);
        cfg->pwm_started = 1;
    }

    // Set CCR (pulse width) = on_ticks (capped at ARR to avoid over 100% duty)
    uint32_t ccr = clamp_u32(on_ticks - 1u, 0u, arr);
    __HAL_TIM_SET_COMPARE(cfg->htim, cfg->channel, ccr);

    // Update last_duty for reference (approximate percentage)
    uint32_t duty_percent_approx = (100u * on_ticks) / total_ticks;
    cfg->last_duty = (uint8_t)clamp_u32(duty_percent_approx, 0u, 100u);

    // Force registers reload immediately (safe with ARPE enabled)
    HAL_TIM_GenerateEvent(cfg->htim, TIM_EVENTSOURCE_UPDATE);
}