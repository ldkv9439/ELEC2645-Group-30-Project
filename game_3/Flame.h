/**
 * @file Flame.h
 * @brief Simple flame object implementation 
 * Controls the flame position and rendering.
 */

#ifndef FLAME_H
#define FLAME_H

#include <stdint.h>
#include "Character.h"

// ===== CONSTANTS =====

// ---- Animation ----
#define FLAME_FRAME_COUNT 2  // Flame animation frames

// ---- Limits ----
#define FLAME_MAX 4          // Maximum number of flames

// ---- Size ----
#define FLAME_COLLISION_RADIUS 8 // Flame radius used to check collision with character
#define FLAME_ADD_RADIUS 25      // Flame radius used when spawning coins (+13 from original radius)
#define FLAME_SIZE_HALF 12       // Half of flame size (8x8 scaled to 3)

// ===== DATA STRUCTURES =====

/**
 * @struct Flame_t
 * @brief Flame object containing position and flag when its active
 */
typedef struct {
    int x;              // Flame X position
    int y;              // Flame Y position
    uint8_t active;    // Flame activation flag
} Flame_t;

/**
 * @struct Coordinates_t
 * @brief Setting coordinates of flame
 */
typedef struct {
    int x;              // Flame X coordinate
    int y;              // Flame Y coordinate
} Flame_Coordinates_t;

// ===== FUNCTIONS =====

/**
 * @brief Reset the number of flame remaining and clear flame
 */
void Flame_Reset (void);

/**
 * @brief Add flame based on amount set
 */
void Flame_Add (int amount);

/**
 * @brief Update number of flame remaining
 * Character's amount of shield and life left depends based on the collision of it with the flame
 * @param character Pointer to character object
 */
void Flame_Update (Character_t* character);

/**
 * @brief Draw flame on LCD
 * Draws the coins as a filled rectangle.
*/
void Flame_Draw (void);

extern Flame_t flame [FLAME_MAX];

#endif