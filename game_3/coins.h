/**
 * @file coins.h
 * @brief Simple coin object implementation 
 * Controls the coins position and rendering.
 */

#ifndef COINS_H
#define COINS_H

#include "Character.h"

#include <stdint.h>

// ===== CONSTANTS =====

// ---- Boundaries ----
#define COINS_MIN_X 20      // Minimum x-axis boundary
#define COINS_MAX_X 200     // Maximum x-axis boundary
#define COINS_MIN_Y 70      // Minimum y-axis boundary
#define COINS_MAX_Y 200     // Maximum y-axis boundary

// ---- Limits ----
#define COINS_MAX 10        // Maximum number of coins

// ---- Size ----
#define COINS_RADIUS 4      // Radius of coins
#define COINS_SPACING 20    // Spacing between coins spawned

// ===== DATA STRUCTURES =====

/**
 * @struct Coins_t
 * @brief Coins object containing position and flag when character collects it
 */
typedef struct {
    int16_t x;              // Coins X position
    int16_t y;              // Coins Y position
    uint8_t collect;        // Check if a coin has been collected
} Coins_t;

// ===== FUNCTIONS =====

/**
 * @brief Reset the number of coins remaining and set all coins have been collected
 */
void Coins_Reset(void);

/**
 * @brief Add coins based on amount set
 */
void Coins_Add(int amount);

/**
 * @brief Update number of coins remaining and the curent score
 * Coins remaining is determined according to the collision between the character and the coins
 * Score increases every time a coins is collected
 * 
 * @param character Pointer to character object
 */
void Coins_Update(Character_t* character);

/**
 * @brief Draw coins on LCD
 * Draws the coins as a filled circle.
*/
void Coins_Draw(void);

// Collision checker
uint8_t Circle_Overlap(uint16_t x1, uint16_t y1, uint16_t r1, uint16_t x2, uint16_t y2, uint16_t r2);

extern int coins_remaining;
extern uint16_t score;

#endif