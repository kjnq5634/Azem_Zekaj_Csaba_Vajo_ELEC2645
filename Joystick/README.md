# Joystick Library

User-friendly joystick driver for the Nucleo L476RG with circle mapping and polar coordinate support.

## Features

- Configurable ADC channels for flexible hardware setup
- Circle mapping for uniform control feel across full input range
- Both cartesian (x,y) and polar (magnitude, angle) coordinate representations
- Compass-style heading (0° = North, increasing clockwise)
- 8-direction discrete output plus continuous angle/magnitude
- Struct-based configuration like the LCD library

## Requirements

- stm32l4xx_hal.h and ADC HAL drivers

## Setup

Add the joystick source files to your CMakeLists.txt:

```cmake
target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/joystick/joystick.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/joystick/
)
```

## ADC Channel Configuration

The joystick library uses ADC channel numbers (e.g., `ADC_CHANNEL_1`), not pin names (e.g., `A1_Pin`).

### STM32L476RG ADC1 Pin to Channel Mapping

| Pin Name | Physical Pin | ADC Channel Constant |
|----------|-------------|---------------------|
| A5       | PC0         | `ADC_CHANNEL_1`     |
| A4       | PC1         | `ADC_CHANNEL_2`     |
| A0       | PA0         | `ADC_CHANNEL_5`     |
| A1       | PA1         | `ADC_CHANNEL_6`     |
| A2       | PA4         | `ADC_CHANNEL_9`     |
| A3       | PB0         | `ADC_CHANNEL_15`    |

**Important:** Pin names (A0-A5) are Arduino-style labels and do not correspond to ADC channel numbers. Always use the ADC channel constant from the table above when configuring the joystick.

### Available ADC Channel Constants

```c
ADC_CHANNEL_1    // PC0 (A5)
ADC_CHANNEL_2    // PC1 (A4)
ADC_CHANNEL_5    // PA0 (A0)
ADC_CHANNEL_6    // PA1 (A1)
ADC_CHANNEL_9    // PA4 (A2)
ADC_CHANNEL_15   // PB0 (A3)
```

## Usage

Configure and initialise the joystick:

```c
Joystick_cfg_t joy_cfg = {
    .adc = &hadc1,
    .x_channel = ADC_CHANNEL_1,  // PC0 (A5_Pin)
    .y_channel = ADC_CHANNEL_2,  // PC1 (A4_Pin)
    .sampling_time = ADC_SAMPLETIME_2CYCLES_5,
    .center_x = JOYSTICK_DEFAULT_CENTER_X,
    .center_y = JOYSTICK_DEFAULT_CENTER_Y,
    .deadzone = JOYSTICK_DEADZONE,
    .setup_done = 0
};

Joystick_t joy_data;
Joystick_Init(&joy_cfg);
Joystick_Calibrate(&joy_cfg);
```

In your main loop:

```c
Joystick_Read(&joy_cfg, &joy_data);

// Get high-level input
UserInput input = Joystick_GetInput(&joy_data);
// Use: input.direction, input.magnitude, input.angle

// Or get full polar coordinates
Polar polar = Joystick_GetPolar(&joy_data);
// Use: polar.mag, polar.angle

// Or use raw Cartesian coordinates
float x = joy_data.coord.x;  // -1.0 to 1.0
float y = joy_data.coord.y;  // -1.0 to 1.0
```

See joystick.h for full API documentation and examples.
