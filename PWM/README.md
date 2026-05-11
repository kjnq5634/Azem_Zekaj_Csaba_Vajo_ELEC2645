# General-Purpose PWM Library

A flexible PWM library for STM32L4 microcontrollers for driving LEDs, servos, motors, and other PWM-controlled devices. Uses a struct-based configuration approach matching the Buzzer and Joystick libraries.

**Key Difference from Buzzer Library:** This library separates frequency and duty cycle control. You set the frequency once, then adjust duty without restarting PWM or changing frequency. Perfect for LED brightness, servo position, or motor speed control.

## Features

- Configuration struct approach for flexible timer/channel selection
- Support for any timer and channel combination
- Configurable frequency range
- Independent duty cycle control (0-100%)
- **Direct tick control** - specify on/off times in timer ticks for advanced applications
- Frequency changes preserve duty cycle
- Clean on/off state management
- Ideal for LEDs, servos, motors, heaters

## Requirements

- stm32l4xx_hal.h and Timer HAL drivers
- Timer must be initialised by CubeMX (MX_TIMx_Init)

## Setup

Add the PWM source files to your CMakeLists.txt:

```cmake
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/PWM/PWM.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/PWM/
)
```

## Hardware Configuration

### Timer Setup (CubeMX)

1. Enable desired timer (e.g., TIM4, TIM3)
2. Configure PWM mode on desired channel
3. **Set prescaler to produce desired tick frequency** (e.g., 79 for 1MHz with 80MHz input clock)
4. Enable ARPE (Auto-Reload Preload) for smooth updates

### Pin Configuration

The PWM GPIO pin is configured automatically by CubeMX's MX_GPIO_Init(). Make sure your .ioc file has:
- Timer output pin configured as AF (alternate function)

## Usage

### Basic Configuration

```c
#include "PWM.h"
#include "tim.h"

// Configure PWM on TIM4 Channel 1 with 1MHz timer clock
PWM_cfg_t led_pwm = {
    .htim = &htim4,
    .channel = TIM_CHANNEL_1,
    .tick_freq_hz = 1000000,  // 1MHz timer tick
    .min_freq_hz = 10,        // Minimum 10Hz
    .max_freq_hz = 50000,     // Maximum 50kHz
    .setup_done = 0
};
```

### Initialisation

```c
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM4_Init();  // Initialise timer with CubeMX
    
    PWM_Init(&led_pwm);  // Prepare PWM library
}
```

### Controlling LEDs

```c
// Set frequency to 1kHz (good for LEDs - not visible flicker)
PWM_SetFreq(&led_pwm, 1000);

// Gradually brighten LED
for (int duty = 0; duty <= 100; duty++) {
    PWM_SetDuty(&led_pwm, duty);
    HAL_Delay(20);
}

// Turn off
PWM_Off(&led_pwm);
```

### Controlling Servos

```c
// Servo control: typically 50Hz frequency
PWM_SetFreq(&servo_pwm, 50);

// Set servo position via duty cycle
// Typical servo: 5% duty = -90°, 7.5% = 0°, 10% = +90°
PWM_SetDuty(&servo_pwm, 7);    // Centre position
HAL_Delay(500);

PWM_SetDuty(&servo_pwm, 5);    // Left
HAL_Delay(500);

PWM_SetDuty(&servo_pwm, 10);   // Right
HAL_Delay(500);
```

### Frequency and Duty Examples

| Application | Frequency | Duty Range | Purpose |
|-------------|-----------|-----------|---------|
| LED breathing | 1-5 Hz | 0-100% | Fade in/out effect |
| LED brightness | 1 kHz | 0-100% | Perceived brightness |
| PWM buzzer | 100-10000 Hz | 0-100% | Tone frequency |
| Servo | 50 Hz | 5-10% | Position control |
| Motor speed | 10-20 kHz | 0-100% | Continuous speed |

### Convenience Function

Set both frequency and duty in one call:

```c
// Set to 1kHz and 75% duty
PWM_Set(&led_pwm, 1000, 75);
```

### Advanced: Direct Tick Control

For advanced applications requiring precise timing control, use `PWM_SetTicks()` to specify the exact number of timer ticks for on and off times:

```c
// Direct control: set on_ticks and off_ticks
// PWM frequency = tick_freq_hz / (on_ticks + off_ticks)
// Duty cycle = on_ticks / (on_ticks + off_ticks)

// Example: With 1MHz timer clock:
// 1kHz PWM at 25% duty:
//   - Total period: 1,000,000 / 1000 = 1000 ticks
//   - on_ticks = 250 (25% of 1000)
//   - off_ticks = 750 (75% of 1000)
PWM_SetTicks(&led_pwm, 250, 750);

// Asymmetric PWM: short pulse, long delay
// Useful for special effects or advanced motor control
PWM_SetTicks(&servo_pwm, 50, 950);  // 5% duty at 1kHz
```

## PWM Frequency and Duty Cycle Calculations

### ARR (Auto-Reload Register) - Frequency Control

The PWM frequency is determined by:

$$f_{PWM} = \frac{f_{tick}}{ARR + 1}$$

Where:
- $f_{PWM}$ = desired output frequency (Hz)
- $f_{tick}$ = timer tick frequency after prescaler (Hz)
- $ARR$ = Auto-Reload Register value

**To calculate ARR for a desired frequency:**

$$ARR = \frac{f_{tick}}{f_{PWM}} - 1$$

**Example: 1kHz PWM with 1MHz timer tick**

$$ARR = \frac{1,000,000}{1000} - 1 = 999$$

This creates a period of 1000 ticks, producing exactly 1kHz output.

### CCR (Compare Register) - Duty Cycle Control

Duty cycle is controlled via the Compare register:

$$\text{Duty Cycle} = \frac{CCR}{ARR + 1} \times 100\%$$

**Example: 50% duty with ARR=999**

$$CCR = (999 + 1) \times \frac{50}{100} = 500$$

This produces a 500/1000 = 50% duty cycle.

**Full brightness (100%):**
$$CCR = ARR = 999$$

**Brightness turned off (0%):**
PWM stops completely, CCR = 0

### Frequency Range

Timer ARR registers are typically 16-bit (max 65535):

**With 1MHz timer tick:**
- Minimum frequency: $\frac{1,000,000}{65536} \approx 15Hz$
- Maximum frequency: $\frac{1,000,000}{1} = 1MHz$

For lower frequencies, increase prescaler in CubeMX.

## Multiple PWM Outputs

Easy to manage multiple independent PWM outputs:

```c
// LED 1
PWM_cfg_t led1 = {
    .htim = &htim4,
    .channel = TIM_CHANNEL_1,
    .tick_freq_hz = 1000000,
    .min_freq_hz = 10,
    .max_freq_hz = 50000,
    .setup_done = 0
};

// LED 2
PWM_cfg_t led2 = {
    .htim = &htim4,
    .channel = TIM_CHANNEL_2,
    .tick_freq_hz = 1000000,
    .min_freq_hz = 10,
    .max_freq_hz = 50000,
    .setup_done = 0
};

// Servo on different timer
PWM_cfg_t servo = {
    .htim = &htim3,
    .channel = TIM_CHANNEL_1,
    .tick_freq_hz = 1000000,
    .min_freq_hz = 10,
    .max_freq_hz = 200,
    .setup_done = 0
};

PWM_Init(&led1);
PWM_Init(&led2);
PWM_Init(&servo);

// Control independently
PWM_SetFreq(&led1, 1000);
PWM_SetDuty(&led1, 75);

PWM_SetFreq(&servo, 50);
PWM_SetDuty(&servo, 7);
```

## API Reference

### `void PWM_Init(PWM_cfg_t* cfg)`

Initialise the PWM library. Must be called once before using other functions.

### `void PWM_SetFreq(PWM_cfg_t* cfg, uint32_t freq_hz)`

Set PWM frequency. Changes frequency while preserving duty cycle if PWM is running.

**Parameters:**
- `freq_hz`: Desired frequency (clamped to min/max range)

### `void PWM_SetDuty(PWM_cfg_t* cfg, uint8_t duty_percent)`

Set PWM duty cycle (brightness/intensity).

**Parameters:**
- `duty_percent`: 0-100 (0 = off, 100 = full on)

**Note:** 0% duty stops PWM. First non-zero call starts PWM.

### `void PWM_Set(PWM_cfg_t* cfg, uint32_t freq_hz, uint8_t duty_percent)`

Convenience function to set both frequency and duty in one call.

### `void PWM_Off(PWM_cfg_t* cfg)`

Stop PWM output completely (same as PWM_SetDuty with 0%).

### `uint8_t PWM_IsRunning(PWM_cfg_t* cfg)`

Check if PWM is currently active.

**Returns:** 1 if running, 0 if stopped

## Configuration Parameters

| Parameter | Type | Example | Purpose |
|-----------|------|---------|---------|
| `htim` | TIM_HandleTypeDef* | `&htim4` | Timer peripheral |
| `channel` | uint32_t | `TIM_CHANNEL_1` | PWM output channel |
| `tick_freq_hz` | uint32_t | `1000000` | Timer clock after prescaler |
| `min_freq_hz` | uint32_t | `10` | Minimum frequency limit |
| `max_freq_hz` | uint32_t | `50000` | Maximum frequency limit |
| `setup_done` | uint8_t | `0` | Initialisation flag (internal) |
| `pwm_started` | uint8_t | `0` | Running state flag (internal) |
| `last_duty` | uint8_t | `0` | Preserved duty for frequency changes |

## Key Differences from Buzzer Library

| Feature | PWM Library | Buzzer Library |
|---------|-------------|----------------|
| Primary use | LEDs, servos, motors | Sound generation |
| Frequency | Fixed, can be changed | Directly controls pitch |
| Duty cycle limit | 0-100% (full range) | Limited to 50% max |
| Use case | Set frequency, adjust duty | Set frequency and duty together |
| Example | Set 1kHz, then vary brightness | Set 440Hz tone |

## Troubleshooting

### PWM Not Working

1. Verify timer is initialised: `MX_TIM4_Init()` called before `PWM_Init()`
2. Check GPIO pin is configured as alternate function in CubeMX
3. Verify timer tick frequency calculation is correct
4. Test with `PWM_IsRunning()` to verify PWM started

### Frequency Changes Don't Work

1. Verify `tick_freq_hz` matches actual timer clock
2. Check frequency is within `min_freq_hz` to `max_freq_hz` range
3. Ensure ARPE (Auto-Reload Preload) is enabled on timer

### LED Not Changing Brightness

1. Call `PWM_SetFreq()` first with desired frequency (e.g., 1kHz for LEDs)
2. Then use `PWM_SetDuty()` to control brightness
3. PWM only starts when duty > 0%

### Servo Not Responding

1. Verify frequency is 50Hz: `PWM_SetFreq(&servo_pwm, 50);`
2. Try duty values between 5-10% (typical servo range)
3. Some servos need slower frequency - try 40-60Hz range
4. Check servo connections and power supply
