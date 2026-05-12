#include "Joystick.h"
#include <stdlib.h>
#include <math.h>

/**
 * @file joystick.c
 * @brief Implementation of joystick driver with coordinate transformations
 * 
 * This file contains:
 * - ADC initialization and channel switching
 * - Raw value processing (deadzone, normalization)
 * - Cartesian to polar coordinate conversion
 * - Circle mapping for uniform control feel
 * - Discrete 8-direction calculation
 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Conversion factor from radians to degrees
 * 
 * Used: angle_degrees = angle_radians * RAD2DEG
 */
#define RAD2DEG 57.2957795131f

/**
 * @brief 12-bit ADC maximum value (2^12 - 1)
 * 
 * Used for normalization calculations in coordinate transformation.
 */
#define JOYSTICK_ADC_RANGE 4095.0f

void Joystick_Init(Joystick_cfg_t* cfg)
{
    // Initialize ADC if not already done
    if (!cfg->setup_done) {
        // Perform ADC calibration
        HAL_ADCEx_Calibration_Start(cfg->adc, ADC_SINGLE_ENDED);
        
        // Initialize cached ADC configuration (set fields that never change)
        cfg->adc_config.Rank = ADC_REGULAR_RANK_1;
        cfg->adc_config.SamplingTime = cfg->sampling_time;
        cfg->adc_config.SingleDiff = ADC_SINGLE_ENDED;
        cfg->adc_config.OffsetNumber = ADC_OFFSET_NONE;
        cfg->adc_config.Offset = 0;
        
        // Configure both channels with the same settings
        cfg->adc_config.Channel = cfg->x_channel;
        HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
        
        cfg->adc_config.Channel = cfg->y_channel;
        HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
        
        cfg->setup_done = 1;
    }
}

void Joystick_Calibrate(Joystick_cfg_t* cfg)
{
    // Take multiple readings and average them to find center position
    uint32_t x_sum = 0, y_sum = 0;
    const int calibration_samples = 50;
    
    for (int i = 0; i < calibration_samples; i++) {
        // Read X channel (use cached config, only change channel)
        cfg->adc_config.Channel = cfg->x_channel;
        HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
        
        HAL_ADC_Start(cfg->adc);
        HAL_ADC_PollForConversion(cfg->adc, HAL_MAX_DELAY);
        x_sum += HAL_ADC_GetValue(cfg->adc);
        HAL_ADC_Stop(cfg->adc);
        
        // Read Y channel (use cached config, only change channel)
        cfg->adc_config.Channel = cfg->y_channel;
        HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
        
        HAL_ADC_Start(cfg->adc);
        HAL_ADC_PollForConversion(cfg->adc, HAL_MAX_DELAY);
        y_sum += HAL_ADC_GetValue(cfg->adc);
        HAL_ADC_Stop(cfg->adc);
        
        HAL_Delay(10);  // Small delay between samples
    }
    
    // Calculate average center positions and store in config
    cfg->center_x = x_sum / calibration_samples;
    cfg->center_y = y_sum / calibration_samples;
}

void Joystick_Read(Joystick_cfg_t* cfg, Joystick_t* data)
{
    // Read X-axis value (use cached config, only change channel)
    cfg->adc_config.Channel = cfg->x_channel;
    HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
    
    HAL_ADC_Start(cfg->adc);
    HAL_ADC_PollForConversion(cfg->adc, HAL_MAX_DELAY);
    data->x_raw = HAL_ADC_GetValue(cfg->adc);
    HAL_ADC_Stop(cfg->adc);
    
    // Read Y-axis value (use cached config, only change channel)
    cfg->adc_config.Channel = cfg->y_channel;
    HAL_ADC_ConfigChannel(cfg->adc, &cfg->adc_config);
    
    HAL_ADC_Start(cfg->adc);
    HAL_ADC_PollForConversion(cfg->adc, HAL_MAX_DELAY);
    data->y_raw = HAL_ADC_GetValue(cfg->adc);
    HAL_ADC_Stop(cfg->adc);
    
    // Process raw values using calibrated center from config
    data->x_processed = data->x_raw - cfg->center_x;
    data->y_processed = data->y_raw - cfg->center_y;
    
    // Apply deadzone from config
    if (abs(data->x_processed) < cfg->deadzone) {
        data->x_processed = 0;
    }
    if (abs(data->y_processed) < cfg->deadzone) {
        data->y_processed = 0;
    }
    
    // Get raw cartesian coordinates (normalized -1.0 to 1.0)
    data->coord = Joystick_GetCoord(data->x_processed, data->y_processed, cfg->center_x, cfg->center_y);
    
    // Map to circular grid
    data->coord_mapped = Joystick_MapToCircle(data->coord);
    
    // Calculate angle from mapped coordinates
    Polar p = Joystick_GetPolar(data);
    data->angle = p.angle;
    data->magnitude = p.mag;
    
    // Calculate direction from angle and magnitude
    data->direction = Joystick_GetDirection(data->angle, data->magnitude);
}

UserInput Joystick_GetInput(Joystick_t* data)
{
    UserInput input;
    input.direction = data->direction;
    input.magnitude = data->magnitude;
    input.angle = data->angle;
    return input;
}

Direction Joystick_GetDirection(float angle, float magnitude)
{
    // Only centre if angle is invalid AND magnitude is very small
    if (angle < 0.0f || magnitude < 0.05f) {
        return CENTRE;
    }
    
    // Convert angle to 8 cardinal/intercardinal directions
    // 0° = North, 45° = NE, 90° = East, etc.
    if (angle >= 337.5f || angle < 22.5f) return N;
    else if (angle >= 22.5f && angle < 67.5f) return NE;
    else if (angle >= 67.5f && angle < 112.5f) return E;
    else if (angle >= 112.5f && angle < 157.5f) return SE;
    else if (angle >= 157.5f && angle < 202.5f) return S;
    else if (angle >= 202.5f && angle < 247.5f) return SW;
    else if (angle >= 247.5f && angle < 292.5f) return W;
    else return NW;
}

// Convert processed ADC values to cartesian coordinates in range -1.0 to 1.0
// Direction (x,y)
// North     (0,1)
// East      (1,0)
// South     (0,-1)
// West      (-1,0)
/**
 * @brief Normalize processed ADC values to Cartesian coordinates [-1.0, 1.0]
 * 
 * @param x Processed X value (centered around 0)
 * @param y Processed Y value (centered around 0)
 * @param center_x Calibrated center position for X (not used after centering, but parameter kept for API)
 * @param center_y Calibrated center position for Y (not used after centering, but parameter kept for API)
 * @return Vector2D with normalized coordinates (-1.0 left to +1.0 right for x, -1.0 down to +1.0 up for y)
 * 
 * @details Converts processed ADC values (which are already centered) to normalized coordinates.
 * 
 * The coordinate system uses a standard compass orientation:
 * - +X axis points East (right): x ranges from -1.0 (left) to +1.0 (right)
 * - +Y axis points North (up): y ranges from -1.0 (down) to +1.0 (up)
 * 
 * The function clamps output to prevent exceeding [-1.0, 1.0] range due to manufacturing variations.
 */
Vector2D Joystick_GetCoord(int16_t x, int16_t y, uint16_t center_x, uint16_t center_y)
{
    // Normalize to -1.0 to 1.0 range based on center
    float norm_x = (float)x / (float)center_x;
    float norm_y = (float)y / (float)center_y;
    
    // Clamp to -1.0 to 1.0
    if (norm_x > 1.0f) norm_x = 1.0f;
    if (norm_x < -1.0f) norm_x = -1.0f;
    if (norm_y > 1.0f) norm_y = 1.0f;
    if (norm_y < -1.0f) norm_y = -1.0f;
    
    // Note: Y is negated so positive is up
    Vector2D coord = {norm_x, -norm_y};
    return coord;
}

// Map raw x,y coordinates onto a circular grid
// See: http://mathproofs.blogspot.co.uk/2005/07/mapping-square-to-circle.html
/**
 * @brief Apply circle mapping to Cartesian coordinates for uniform control feel
 * 
 * @param coord Input Cartesian coordinates (-1.0 to 1.0)
 * @return Vector2D with circle-mapped coordinates (-1.0 to 1.0)
 * 
 * @details Circle mapping transforms the square input range into a circular output range.
 * This ensures that users feel equal resistance pushing the stick in all directions,
 * and maximizes the effective range of control at the diagonal corners.
 * 
 * **Algorithm:**
 * The mapping formula is:
 * - x' = x * sqrt(1 - y²/2)
 * - y' = y * sqrt(1 - x²/2)
 * 
 * This is the "square to circle" transformation, which preserves the magnitude
 * of the input vector while stretching the corners outward.
 * 
 * **Effect:**
 * - At corners (±1, ±1): magnitude decreases from √2 to 1.0
 * - At cardinal directions (±1, 0) or (0, ±1): magnitude unchanged at 1.0
 * - At origin (0, 0): magnitude stays 0.0
 * 
 * **Reference:** http://mathproofs.blogspot.co.uk/2005/07/mapping-square-to-circle.html
 */
Vector2D Joystick_MapToCircle(Vector2D coord)
{
    float x = coord.x * sqrtf(1.0f - (coord.y * coord.y) / 2.0f);
    float y = coord.y * sqrtf(1.0f - (coord.x * coord.x) / 2.0f);
    
    Vector2D mapped = {x, y};
    return mapped;
}

// Get polar coordinates (magnitude and angle) with circle mapping
// Angle is in degrees 0-360, or -1 if centered
/**
 * @brief Calculate polar coordinates from circle-mapped joystick position
 * 
 * @param data Pointer to populated joystick data struct (must have coord_mapped set)
 * @return Polar struct with magnitude (0.0-1.0) and angle (0-360° or -1 if centered)
 * 
 * @details Converts Cartesian coordinates to polar form for easier magnitude and heading control.
 * 
 * **Coordinate System (Compass Heading):**
 * - 0° = North (up): point (0, 1)
 * - 90° = East (right): point (1, 0)
 * - 180° = South (down): point (0, -1)
 * - 270° = West (left): point (-1, 0)
 * 
 * Angle increases clockwise when viewed from above (standard compass convention).
 * 
 * **Magnitude:**
 * - 0.0 = stick at center
 * - 1.0 = stick fully deflected (at cardinal directions)
 * 
 * **Center Detection:**
 * If magnitude is effectively zero (deadzone already filtered raw values),
 * angle is set to -1.0 as an invalid marker. Center filtering happens
 * in Joystick_Read() via the deadzone parameter.
 * 
 * **Algorithm:**
 * 1. Swap axes and negate y to convert from mathematical angle (0°=East) to compass angle (0°=North)
 * 2. Calculate magnitude using Pythagorean theorem: mag = sqrt(x² + y²)
 * 3. Calculate angle using arctangent: angle = atan2(y, x) converted to degrees
 * 4. Convert from [-180°, 180°] range to [0°, 360°] range
 * 5. Check center detection threshold
 */
Polar Joystick_GetPolar(Joystick_t* data)
{
    Polar p;
    
    // At this point, 0 degrees (x-axis) is East
    // We want 0 degrees to correspond to North and increase clockwise
    // like a compass heading, so swap axes and invert y
    float x = data->coord_mapped.y;
    float y = data->coord_mapped.x;
    
    float mag = sqrtf(x * x + y * y);  // Pythagorean theorem
    float angle = RAD2DEG * atan2f(y, x);
    
    // Convert angle from -180 to 180 range to 0 to 360 range
    if (angle < 0.0f) {
        angle += 360.0f;
    }
    
    // If centered (deadzone already applied), mark angle as invalid
    if (mag < 0.01f) {  // Near-zero check for floating point
        angle = -1.0f;  // Invalid angle indicates centered
    }
    
    p.mag = mag;
    p.angle = angle;
    return p;
}

