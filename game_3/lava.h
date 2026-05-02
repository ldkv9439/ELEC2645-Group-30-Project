/**
 * @file lava.h
 * @brief Simple coin object implementation 
 * Controls the coins position and rendering.
 */

#ifndef LAVA_H
#define LAVA_H

#include "Joystick.h"
#include "LCD.h"
#include <stdint.h>
#include <stdlib.h>
#include "Character.h"

#define LAVA_MAX 5    // Maximum number of coins
#define LAVA_SIZE 20  // Radius of coins
#define LAVA_RADIUS 10  // Radius of coins

/**
 * @struct Lava_t
 * @brief Lava object containing position and flag when its active
 */
typedef struct {
    int x;              // Lava X position
    int y;              // Lava Y position
    uint8_t active;    // Lava activation flag
} Lava_t;

/**
 * @struct Coordinates_t
 * @brief Setting coordinates of lava
 */
typedef struct {
    int x;              // Lava X coordinate
    int y;              // Lava Y coordinate
} Lava_Coordinates_t;

/**
 * @brief Reset the number of lava remaining and clear lava
 */
void Lava_Reset (void);

/**
 * @brief Add lava based on amount set
 */
void Lava_Add (int amount);

/**
 * @brief Update number of lava remaining
 * Character's amount of shield and life left depends based on the collision of it with the lava
 * @param character Pointer to character object
 */
void Lava_Update (Character_t* character);

/**
 * @brief Draw lava on LCD
 * Draws the coins as a filled rectangle.
*/
void Lava_Draw (void);
extern Lava_t lava [LAVA_MAX];

#endif