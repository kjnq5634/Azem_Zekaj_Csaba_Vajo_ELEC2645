# Buzzer Library

A flexible PWM-based buzzer library for STM32L4 microcontrollers with frequency and volume control via PWM. Uses a struct-based configuration approach matching the Joystick and LCD libraries for consistency.

## Features

- Configuration struct approach for flexible timer/channel selection
- Support for any timer and channel combination
- Configurable frequency range with clamping
- Volume control (0-100%)
- **Musical note support (C4-C8)** - play notes directly instead of Hz values (including sharps/flats)
- Auto-initialization on first use
- Clean on/off state management
- Detailed PWM frequency and duty cycle calculations

## Requirements

- stm32l4xx_hal.h and Timer HAL drivers
- Timer must be initialized by CubeMX (MX_TIMx_Init)

## Setup

Add the buzzer source files to your CMakeLists.txt:

```cmake
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Buzzer/Buzzer.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/Buzzer/
)
```

## Hardware Configuration

### Timer Setup (CubeMX)

1. Enable desired timer (e.g., TIM2, TIM3, TIM4)
2. Configure PWM mode on desired channel
3. **Set prescaler to produce desired tick frequency** (e.g., 79 for 1MHz with 80MHz input clock)
4. Enable ARPE (Auto-Reload Preload) for smooth frequency updates

### Pin Configuration

The buzzer GPIO pin is configured automatically by CubeMX's MX_GPIO_Init(). Make sure your .ioc file has:
- Timer output pin configured as AF (alternate function)
- No special GPIO configuration needed in the buzzer library

## Usage

### Basic Configuration

```c
#include "buzzer.h"
#include "tim.h"

// Configure buzzer on TIM2 Channel 3 with 1MHz timer clock
Buzzer_cfg_t buzzer_cfg = {
    .htim = &htim2,
    .channel = TIM_CHANNEL_3,
    .tick_freq_hz = 1000000,  // Timer tick frequency after prescaler
    .min_freq_hz = 20,
    .max_freq_hz = 20000,
    .setup_done = 0
};
```

### Initialization

```c
int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();  // Initialize timer with CubeMX
    
    buzzer_init(&buzzer_cfg);  // Prepare buzzer library
}
```

### Playing Tones

```c
// Play 440Hz (A4 musical note) at 50% volume
buzzer_tone(&buzzer_cfg, 440, 50);

// Play a musical note directly (C4 to C8, including sharps)
buzzer_note(&buzzer_cfg, NOTE_C4, 50);
buzzer_note(&buzzer_cfg, NOTE_A4, 50);  // Standard A4 (440Hz)
buzzer_note(&buzzer_cfg, NOTE_C5, 50);
buzzer_note(&buzzer_cfg, NOTE_CS5, 50); // C#5
buzzer_note(&buzzer_cfg, NOTE_C8, 50);  // C8

// Play a beep
buzzer_tone(&buzzer_cfg, 1000, 60);
HAL_Delay(500);

// Stop the buzzer
buzzer_off(&buzzer_cfg);

// Check if buzzer is running
if (buzzer_is_running(&buzzer_cfg)) {
    printf("Buzzer is active\n");
}
```

### Playing a Musical Scale

```c
// Play C major scale using musical notes (Octave 4)
int scale[] = {NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5};

for (int i = 0; i < 8; i++) {
    buzzer_note(&buzzer_cfg, scale[i], 50);
    HAL_Delay(300);
}
buzzer_off(&buzzer_cfg);

// Play notes with sharps (chromatic scale from C4 to C5)
int chromatic[] = {
    NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4,
    NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4, NOTE_C5
};

for (int i = 0; i < 13; i++) {
    buzzer_note(&buzzer_cfg, chromatic[i], 50);
    HAL_Delay(200);
}
buzzer_off(&buzzer_cfg);
```

### Musical Note Frequencies and Enums

**Octave 4 (Middle C and above):**

| Note | Enum | Frequency (Hz) |
|------|------|---|
| C4 | NOTE_C4 | 262 |
| C#4/Db4 | NOTE_CS4 | 277 |
| D4 | NOTE_D4 | 294 |
| D#4/Eb4 | NOTE_DS4 | 311 |
| E4 | NOTE_E4 | 330 |
| F4 | NOTE_F4 | 349 |
| F#4/Gb4 | NOTE_FS4 | 370 |
| G4 | NOTE_G4 | 392 |
| G#4/Ab4 | NOTE_GS4 | 415 |
| A4 | NOTE_A4 | 440 |
| A#4/Bb4 | NOTE_AS4 | 466 |
| B4 | NOTE_B4 | 494 |

**Octave 5:**

| Note | Enum | Frequency (Hz) |
|------|------|---|
| C5 | NOTE_C5 | 523 |
| C#5/Db5 | NOTE_CS5 | 554 |
| D5 | NOTE_D5 | 587 |
| D#5/Eb5 | NOTE_DS5 | 622 |
| E5 | NOTE_E5 | 659 |
| F5 | NOTE_F5 | 698 |
| F#5/Gb5 | NOTE_FS5 | 740 |
| G5 | NOTE_G5 | 784 |
| G#5/Ab5 | NOTE_GS5 | 831 |
| A5 | NOTE_A5 | 880 |
| A#5/Bb5 | NOTE_AS5 | 932 |
| B5 | NOTE_B5 | 988 |

**Octave 6:**

| Note | Enum | Frequency (Hz) |
|------|------|---|
| C6 | NOTE_C6 | 1047 |
| C#6/Db6 | NOTE_CS6 | 1109 |
| D6 | NOTE_D6 | 1175 |
| D#6/Eb6 | NOTE_DS6 | 1245 |
| E6 | NOTE_E6 | 1319 |
| F6 | NOTE_F6 | 1397 |
| F#6/Gb6 | NOTE_FS6 | 1480 |
| G6 | NOTE_G6 | 1568 |
| G#6/Ab6 | NOTE_GS6 | 1661 |
| A6 | NOTE_A6 | 1760 |
| A#6/Bb6 | NOTE_AS6 | 1865 |
| B6 | NOTE_B6 | 1976 |

**Octave 7:**

| Note | Enum | Frequency (Hz) |
|------|------|---|
| C7 | NOTE_C7 | 2093 |
| C#7/Db7 | NOTE_CS7 | 2217 |
| D7 | NOTE_D7 | 2349 |
| D#7/Eb7 | NOTE_DS7 | 2489 |
| E7 | NOTE_E7 | 2637 |
| F7 | NOTE_F7 | 2794 |
| F#7/Gb7 | NOTE_FS7 | 2960 |
| G7 | NOTE_G7 | 3136 |
| G#7/Ab7 | NOTE_GS7 | 3322 |
| A7 | NOTE_A7 | 3520 |
| A#7/Bb7 | NOTE_AS7 | 3729 |
| B7 | NOTE_B7 | 3951 |

**Octave 8:**

| Note | Enum | Frequency (Hz) |
|------|------|---|
| C8 | NOTE_C8 | 4186 |

## PWM Frequency and Duty Cycle Calculations

This section explains the maths behind the buzzer's PWM signal generation.

### ARR (Auto-Reload Register) - Frequency Control

The PWM frequency is determined by the ARR register:

$$f_{PWM} = \frac{f_{tick}}{ARR + 1}$$

Where:
- $f_{PWM}$ = desired output frequency (Hz)
- $f_{tick}$ = timer tick frequency after prescaler (Hz)
- $ARR$ = Auto-Reload Register value

**To calculate ARR for a desired frequency:**

$$ARR = \frac{f_{tick}}{f_{PWM}} - 1$$

**Example: Playing 440Hz with 1MHz timer tick**

$$ARR = \frac{1,000,000}{440} - 1 = 2273 - 1 = 2272$$

This creates a timer period of 2273 ticks, which at 1MHz produces:
$$f_{PWM} = \frac{1,000,000}{2273} \approx 440\text{ Hz}$$

### CCR (Compare Register) - Duty Cycle / Volume Control

The duty cycle determines the "loudness" and waveform shape:

$$\text{Duty Cycle} = \frac{CCR}{ARR + 1} \times 100\%$$

The buzzer library limits duty cycle to **50% maximum** for a cleaner square-wave tone:

$$CCR_{max} = \frac{ARR + 1}{2}$$

**Volume mapping (0-100%):**

$$CCR = \frac{(ARR + 1)}{2} \times \frac{\text{volume}}{100}$$

**Example: 440Hz at 50% volume with ARR=2272**

$$CCR_{max} = \frac{2273}{2} = 1136.5 \approx 1136$$
$$CCR_{50\%} = 1136 \times \frac{50}{100} = 568$$

This produces a duty cycle of $\frac{568}{2273} \approx 25\%$ (half of the 50% maximum), creating a perceptually balanced tone.

### Timer Prescaler Configuration

The prescaler determines the tick frequency from the input clock:

$$f_{tick} = \frac{f_{input}}{Prescaler + 1}$$

**For STM32L476RG with 80MHz system clock on TIM2:**

To achieve 1MHz tick frequency:
$$Prescaler = \frac{80,000,000}{1,000,000} - 1 = 79$$

Set this in CubeMX when configuring the timer. You can also see this setup in the code in `tim.h`.

### Frequency Range vs ARR Limits

Timer ARR registers are typically 16-bit or 32-bit, limiting the range of frequencies:

**For 16-bit timer (max ARR = 65535):**
- Minimum frequency: $\frac{1,000,000}{65536} \approx 15\text{ Hz}$
- Maximum frequency: $\frac{1,000,000}{1} = 1\text{ MHz}$

**For 32-bit timer (max ARR = 4,294,967,295):**
- Much wider range
- Limited mainly by the `max_freq_hz` configuration

## Multiple Buzzers

The configuration struct approach allows multiple buzzers on different timers:

```c
// Buzzer on TIM2
Buzzer_cfg_t buzzer1 = {
    .htim = &htim2,
    .channel = TIM_CHANNEL_3,
    .tick_freq_hz = 1000000,
    .min_freq_hz = 20,
    .max_freq_hz = 20000,
    .setup_done = 0
};

// Buzzer on TIM3
Buzzer_cfg_t buzzer2 = {
    .htim = &htim3,
    .channel = TIM_CHANNEL_1,
    .tick_freq_hz = 500000,  // Different tick frequency
    .min_freq_hz = 20,
    .max_freq_hz = 10000,
    .setup_done = 0
};

buzzer_init(&buzzer1);
buzzer_init(&buzzer2);

buzzer_tone(&buzzer1, 440, 50);
buzzer_tone(&buzzer2, 880, 50);  // Different frequency on different timer
```

## GPIO Pin Initialization Order

**Important:** When using LCD (ST7789V2) and buzzer together on GPIOB:

1. Initialize LCD FIRST (configures GPIOB pins)
2. Initialize buzzer SECOND (PB10 AF1 configuration layers on LCD GPIO setup)

This ensures both peripherals work correctly:

```c
// Initialize LCD first
LCD_init(&cfg0);

// Then initialize buzzer
buzzer_init(&buzzer_cfg);
```

If you use different GPIO ports, initialization order doesn't matter.

## API Reference

### `void buzzer_init(Buzzer_cfg_t* cfg)`

Initialize the buzzer library. Must be called once before using `buzzer_tone()`.

### `void buzzer_tone(Buzzer_cfg_t* cfg, uint32_t freq_hz, uint8_t volume_percent)`

Play a tone on the buzzer.
- `freq_hz`: Frequency in Hz (clamped to min/max range)
- `volume_percent`: 0-100 (0 = off, 100 = maximum)

### `void buzzer_note(Buzzer_cfg_t* cfg, Buzzer_Note_t note, uint8_t volume_percent)`

Play a musical note on the buzzer.
- `note`: Musical note enum (C4 to C8, including sharps/flats)
- `volume_percent`: 0-100 (0 = off, 100 = maximum)

### `void buzzer_off(Buzzer_cfg_t* cfg)`

Stop the buzzer completely (PWM stopped, output disabled).

### `uint8_t buzzer_is_running(Buzzer_cfg_t* cfg)`

Check if buzzer is currently playing (PWM active).
- Returns 1 if running, 0 if stopped

## Configuration Parameters

| Parameter | Type | Example | Purpose |
|-----------|------|---------|---------|
| `htim` | TIM_HandleTypeDef* | `&htim2` | Timer peripheral |
| `channel` | uint32_t | `TIM_CHANNEL_3` | PWM output channel |
| `tick_freq_hz` | uint32_t | `1000000` | Timer clock after prescaler |
| `min_freq_hz` | uint32_t | `20` | Minimum audible frequency |
| `max_freq_hz` | uint32_t | `20000` | Maximum audible frequency |
| `setup_done` | uint8_t | `0` | Initialization flag (internal) |
| `pwm_started` | uint8_t | `0` | Running state flag (internal) |

## Troubleshooting

### Buzzer Not Working

1. Verify timer is initialized: `MX_TIM2_Init()` called before `buzzer_init()`
2. Check GPIO pin is configured as alternate function in CubeMX
3. Verify timer tick frequency calculation is correct
4. Test with `buzzer_is_running()` to verify PWM started

### Frequency Not Changing

1. Verify `tick_freq_hz` matches actual timer clock
2. Check frequency is within `min_freq_hz` to `max_freq_hz` range
3. Ensure ARPE (Auto-Reload Preload) is enabled on timer

### Sound Quality

- Adjust `min_freq_hz` and `max_freq_hz` for your piezo buzzer's optimal range
- Adjust volume percent (0-100%) to control perceived loudness
- Duty cycle is limited to 50% for cleaner square-wave tone
