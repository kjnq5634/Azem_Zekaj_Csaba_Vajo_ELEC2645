/**
 * @file Utils.h
 * @brief Common utility structures and types for games
 * 
 * Includes position vectors, input structures, and collision detection types.
 * Uses Direction, Vector2D, and UserInput from Joystick.h
 *
 * NOTE: This header currently contains tiny static inline helpers.
 * If utilities grow more complex, move implementations to a .c file
 * and leave only prototypes here (standard .h/.c style).
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include "Joystick.h"
#include "rng.h"

/* ===== POSITION TYPE ===== */

/**
 * @struct Position2D
 * @brief 2D position with integer coordinates
 */
typedef struct {
    int16_t x;
    int16_t y;
} Position2D;

/* ===== AABB COLLISION DETECTION ===== */

/**
 * @struct AABB
 * @brief Axis-Aligned Bounding Box for collision detection
 * 
 * AABB (Axis-Aligned Bounding Box) is the simplest collision detection method:
 * - Stores a rectangle with position (x, y) and dimensions (width, height)
 * - Boxes are always axis-aligned (no rotation)
 * - Two AABBs collide if they overlap in BOTH X and Y directions
 * 
 * **How AABB Collision Works:**
 * 
 * For two boxes A and B to collide, they must overlap in both axes:
 * - X-axis: A.x < B.x+B.width AND A.x+A.width > B.x
 * - Y-axis: A.y < B.y+B.height AND A.y+A.height > B.y
 * 
 * If either condition is false, boxes do not overlap.
 */
typedef struct {
    int16_t x;        // Top-left X coordinate
    int16_t y;        // Top-left Y coordinate
    int16_t width;    // Width in pixels
    int16_t height;   // Height in pixels
} AABB;

/**
 * @function AABB_Collides
 * @brief Check if two AABBs are colliding
 * 
 * @param a Pointer to first bounding box
 * @param b Pointer to second bounding box
 * @return 1 if boxes collide, 0 otherwise
 * 
 * **Algorithm:**
 * Returns true if boxes overlap in BOTH X AND Y axes simultaneously:
 * ```
 * a.x < b.x + b.width     &&   // A's left < B's right
 * a.x + a.width > b.x     &&   // A's right > B's left
 * a.y < b.y + b.height    &&   // A's top < B's bottom
 * a.y + a.height > b.y         // A's bottom > B's top
 * ```
 * 
 * NOTE: Defined as static inline in a header to avoid duplicate symbol errors.
 * This pattern is best kept for very small helper functions like this one.
 */
static inline uint8_t AABB_Collides(AABB* a, AABB* b) {
    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

/**
 * @brief Generate random number in range [0, max]
 * This function uses the RNG peripheral which returns a pseudorandom number
 * which is more random than the standard library rand() function.
 * 
 * NOTE: Defined as static inline in a header to avoid duplicate symbol errors.
 * This pattern is best kept for very small helper functions like this one.
 */
static inline uint16_t Random_U16(uint16_t max)
{
    uint32_t rnd = 0;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &rnd) != HAL_OK) {
        return 0;
    }
    if (max == 0) {
        return 0;
    }
    return (uint16_t)(rnd % max);
}

#endif // UTILS_H
