#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdint.h>
#include <stdlib.h>
#include "main.h"

/**
 * @file joystick.h
 * @brief User-friendly joystick library with polar coordinates and circle mapping
 * 
 * This library features:
 * - Configurable ADC channels for flexible hardware setup
 * - Circle mapping for uniform control feel across full input range
 * - Both cartesian (x,y) and polar (magnitude, angle) coordinate representations
 * - Compass-style heading (0° = North, increasing clockwise)
 * - Automatic center detection
 * - 8-direction discrete output plus continuous angle/magnitude
 * 
 * The library uses a struct-based configuration approach similar to the LCD library,
 * allowing multiple joysticks or sharing ADC peripherals with other sensors.
 * 
 * Example usage:
 * @code
 * // Configure first joystick on ADC1 channels 1 and 2
 * Joystick_cfg_t joy1_cfg = {
 *     .adc = &hadc1,
 *     .x_channel = ADC_CHANNEL_1,
 *     .y_channel = ADC_CHANNEL_2,
 *     .sampling_time = ADC_SAMPLETIME_2CYCLES_5,
 *     .center_x = JOYSTICK_DEFAULT_CENTER_X,
 *     .center_y = JOYSTICK_DEFAULT_CENTER_Y,
 *     .deadzone = JOYSTICK_DEADZONE,
 *     .setup_done = 0
 * };
 * 
 * Joystick_t joy1_data;
 * Joystick_Init(&joy1_cfg);
 * Joystick_Calibrate(&joy1_cfg);
 * 
 * // In main loop:
 * Joystick_Read(&joy1_cfg, &joy1_data);  // Read and process all coordinates
 * UserInput input = Joystick_GetInput(&joy1_data);  // Get direction and angle
 * Polar polar = Joystick_GetPolar(&joy1_data);      // Get full polar coords
 * @endcode
 */

// Direction enum for joystick input
typedef enum {
    CENTRE = 0,
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
} Direction;

/**
 * @struct UserInput
 * @brief High-level joystick input representation for game/application logic
 * 
 * @details Convenient struct for getting the most commonly used joystick values:
 * - direction: 8-way discrete output for game controls (N, NE, E, SE, S, SW, W, NW, CENTRE)
 * - magnitude: 0.0->1.0 continuous value for speed/intensity control
 * - angle: 0-360° for applications needing fine directional control (or -1 if centered)
 */
typedef struct {
    Direction direction;    ///< 8-direction enum (N, NE, E, SE, S, SW, W, NW, CENTRE)
    float magnitude;        ///< Magnitude 0.0 -> 1.0 (from circle-mapped coordinates)
    float angle;            ///< Raw angle 0-360° for finer control (from circle-mapped coordinates), or -1 if centered
} UserInput;

// Cartesian coordinates (x, y in range -1.0 to 1.0)
// Direction mapping: North=(0,1), East=(1,0), South=(0,-1), West=(-1,0)
/**
 * @struct Vector2D
 * @brief Cartesian coordinates for joystick position
 */
typedef struct {
    float x;    ///< X-axis coordinate (-1.0 left to +1.0 right)
    float y;    ///< Y-axis coordinate (-1.0 down to +1.0 up)
} Vector2D;

// Polar coordinates (magnitude and angle from circle-mapped coordinates)
/**
 * @struct Polar
 * @brief Polar coordinates from circle-mapped joystick position
 */
typedef struct {
    float mag;      ///< Magnitude 0.0 -> 1.0 (from circle-mapped coordinates), or 0 if centered
    float angle;    ///< Angle 0-360° (compass: 0°=North, 90°=East), or -1 if centered
} Polar;

// Joystick configuration defaults
/**
 * @defgroup Joystick_Config Joystick Configuration Defaults
 * @brief Default calibration and configuration values for joystick operation
 * @{
 */
#define JOYSTICK_DEFAULT_CENTER_X 2048  ///< Default center position for X-axis (12-bit ADC: 0-4095, midpoint=2048)
#define JOYSTICK_DEFAULT_CENTER_Y 2048  ///< Default center position for Y-axis (12-bit ADC: 0-4095, midpoint=2048)
#define JOYSTICK_DEADZONE 200           ///< Default deadzone radius around center (in ADC units, ~5% of max range)
#define JOYSTICK_MAX_VALUE 4095         ///< Maximum 12-bit ADC value (used for normalization)
/**
 * @}
 */

// Joystick configuration structure
/**
 * @struct Joystick_cfg_t
 * @brief Configuration for a joystick instance
 * 
 * @details Holds all static configuration that never changes after initialization.
 * Allows multiple joysticks on different ADC channels or sharing the same ADC.
 */
typedef struct {
    ADC_HandleTypeDef* adc;             ///< Pointer to ADC handle (e.g., &hadc1)
    uint32_t x_channel;                 ///< ADC channel for X-axis (e.g., ADC_CHANNEL_1)
    uint32_t y_channel;                 ///< ADC channel for Y-axis (e.g., ADC_CHANNEL_2)
    uint32_t sampling_time;             ///< ADC sampling time (e.g., ADC_SAMPLETIME_2CYCLES_5)
    uint16_t center_x;                  ///< Calibrated center ADC value for X (typically ~2048 for 12-bit)
    uint16_t center_y;                  ///< Calibrated center ADC value for Y (typically ~2048 for 12-bit)
    uint16_t deadzone;                  ///< Deadzone around center in ADC units (e.g., 200)
    uint8_t setup_done;                 ///< Internal flag: 1 if initialized, 0 otherwise
    ADC_ChannelConfTypeDef adc_config;  ///< Cached ADC channel configuration (set during Init)
} Joystick_cfg_t;

// Joystick data structure - populated by Joystick_Read()
/**
 * @struct Joystick_t
 * @brief Current joystick state with all coordinate representations
 * 
 * @details Contains raw ADC values, processed values with deadzone applied,
 * and both Cartesian and polar coordinate systems. Populated by Joystick_Read().
 */
typedef struct {
    uint16_t x_raw;         ///< Raw ADC value for X-axis (0-4095 for 12-bit ADC)
    uint16_t y_raw;         ///< Raw ADC value for Y-axis (0-4095 for 12-bit ADC)
    int16_t x_processed;    ///< Centered X with deadzone applied (-2048 to +2048)
    int16_t y_processed;    ///< Centered Y with deadzone applied (-2048 to +2048)
    Vector2D coord;         ///< Normalized Cartesian coords (-1.0 to 1.0) before circle mapping
    Vector2D coord_mapped;  ///< Circle-mapped coordinates (-1.0 to 1.0) for uniform control
    float angle;            ///< Angle 0-360° from circle-mapped coords (compass: 0°=North, 90°=East), or -1 if centered
    Direction direction;    ///< Discrete 8-direction output (N, NE, E, SE, S, SW, W, NW, CENTRE)
    float magnitude;        ///< Magnitude 0.0->1.0 from circle-mapped coords
} Joystick_t;

// Function prototypes
// Core functions

/**
 * @brief Initialize joystick with ADC configuration
 * 
 * @param cfg Pointer to joystick configuration struct
 * 
 * @details Performs one-time ADC setup including:
 * - Configures ADC channels and sampling time
 * - Builds and caches ADC configuration struct for efficient channel switching
 * - Sets setup_done flag to prevent duplicate initialization
 * 
 * Call this once during system initialization before using Joystick_Read().
 * After Init, use Joystick_Calibrate() to find center position.
 * 
 * @note Must be called exactly once per joystick configuration
 */
void Joystick_Init(Joystick_cfg_t* cfg);

/**
 * @brief Calibrate joystick center position
 * 
 * @param cfg Pointer to joystick configuration struct
 * 
 * @details Reads 50 ADC samples and averages them to find the neutral center position.
 * Stores results in cfg->center_x and cfg->center_y.
 * 
 * Should be called after Joystick_Init() while joystick is held in neutral position.
 * This calibration compensates for manufacturing variations and slight mechanical offsets.
 * 
 * @note Blocking operation (~500ms for 50 samples)
 */
void Joystick_Calibrate(Joystick_cfg_t* cfg);

/**
 * @brief Read joystick and compute all coordinate representations
 * 
 * @param cfg Pointer to joystick configuration struct
 * @param data Pointer to joystick data struct to be populated
 * 
 * @details Performs complete joystick processing:
 * - Reads raw ADC values for both X and Y axes
 * - Applies deadzone around center (values within deadzone → 0)
 * - Normalizes to Cartesian coordinates (-1.0 to 1.0)
 * - Applies circle mapping for uniform control feel
 * - Calculates polar coordinates (magnitude, angle)
 * - Determines 8-direction discrete output
 * 
 * All fields in data struct are populated. Call this in your main loop
 * to update joystick state before reading individual fields.
 * 
 * @note Non-blocking operation (~200μs for two ADC conversions)
 */
void Joystick_Read(Joystick_cfg_t* cfg, Joystick_t* data);

// Data retrieval functions

/**
 * @brief Get high-level joystick input (direction, magnitude, angle)
 * 
 * @param data Pointer to populated joystick data struct
 * @return UserInput struct with direction enum, magnitude, and angle
 * 
 * @details Extracts the most commonly used joystick values:
 * - direction: 8-way discrete output (useful for game controls)
 * - magnitude: 0.0->1.0 from circle-mapped coords (useful for speed control)
 * - angle: 0-360° for finer directional control (or -1 if centered)
 * 
 * This is the primary interface for most applications. Call after Joystick_Read().
 */
UserInput Joystick_GetInput(Joystick_t* data);

/**
 * @brief Get full polar coordinates from circle-mapped position
 * 
 * @param data Pointer to populated joystick data struct
 * @return Polar struct with magnitude and angle
 * 
 * @details Returns the polar representation computed during Joystick_Read().
 * Same data as Joystick_GetInput() polar fields, but in dedicated struct.
 * 
 * Magnitude: 0.0->1.0, where 1.0 is maximum deflection
 * Angle: 0-360°, where 0°=North, 90°=East, etc. (or -1 if centered)
 */
Polar Joystick_GetPolar(Joystick_t* data);

// Helper functions (used internally, available for advanced users)

/**
 * @brief Normalize raw ADC readings to Cartesian coordinates
 * 
 * @param x Raw ADC value for X-axis
 * @param y Raw ADC value for Y-axis
 * @param center_x Calibrated center position for X
 * @param center_y Calibrated center position for Y
 * @return Vector2D with normalized coordinates (-1.0 to 1.0)
 * 
 * @details Subtracts center and scales to [-1.0, 1.0] range based on JOYSTICK_MAX_VALUE.
 * Used internally by Joystick_Read() to normalize coordinates.
 */
Vector2D Joystick_GetCoord(int16_t x, int16_t y, uint16_t center_x, uint16_t center_y);

/**
 * @brief Apply circle mapping transformation
 * 
 * @param coord Cartesian coordinates (-1.0 to 1.0)
 * @return Circle-mapped coordinates (-1.0 to 1.0)
 * 
 * @details Transforms square input range to circular output for uniform control feel.
 * Algorithm: x' = x*sqrt(1 - y²/2), y' = y*sqrt(1 - x²/2)
 * This ensures equal force is needed in all directions and maximizes effective range.
 * 
 * Reference: http://mathproofs.blogspot.co.uk/2005/07/mapping-square-to-circle.html
 */
Vector2D Joystick_MapToCircle(Vector2D coord);

/**
 * @brief Get 8-direction output from angle and magnitude
 * 
 * @param angle Raw angle in degrees (0-360°, or -1 for centered)
 * @param magnitude Magnitude 0.0->1.0
 * @return Direction enum (N, NE, E, SE, S, SW, W, NW, or CENTRE)
 * 
 * @details Maps continuous angle to discrete 8-direction output.
 * Compass orientation: 0°=N, 45°=NE, 90°=E, 135°=SE, etc.
 * Returns CENTRE if angle < 0 or magnitude < 0.05 (within deadzone)
 * 
 * Used internally by Joystick_Read() but available for custom angle inputs.
 */
Direction Joystick_GetDirection(float angle, float magnitude);

#endif /* JOYSTICK_H */
