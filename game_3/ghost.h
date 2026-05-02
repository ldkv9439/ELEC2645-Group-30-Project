/**
 * @file ghost.h
 * @brief Simple ghost sprite for demo purposes
 * Controls the ghosts and bullets position and rendering.
 */
 
#ifndef GHOST_H
#define GHOST_H

#include "Joystick.h"
#include "LCD.h"
#include "stm32l4xx_hal.h"
#include "Character.h"

#include <stdint.h>
#include <stdlib.h>

#define GHOST_FRAME_COUNT 2     // Ghost animation frames
#define GHOST_MAX 5             // Maximum number of ghosts
#define GHOST_MAX_BULLETS 10    // Maximum number of bullets
#define LIFE_MAX 7              // Maximum number of character's life
#define SHIELD_MAX 5            // Maximum number of character's shield

/**
 * @struct Ghost_t
 * @brief Ghost object containing position
 */
typedef struct {
    int x;
    int y;
} Ghost_t;

/**
 * @struct Bullet_t
 * @brief Bullet object containing position, movements and flag if bullet is valid
 */
typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    uint8_t valid;
} Bullet_t;

/**
 * @brief Reset the number of ghosts remaining
 */
void Ghost_Reset(void);

/**
 * @brief Add ghosts based on coordinates set
 */
void Ghost_Add(int x, int y);

/**
 * @brief Update the ghosts action of shooting the bullets
 * Character's amount of shield and life left depends based on the collision of it with the bullets shot
 * Bullets bounce off the wall 
 * Shield can recharge
 * 
 * @param character Pointer to character object
 */
void Ghost_Update(Character_t* character);

/**
 * @brief Determine direction of bullets
 */
void Bullet_Update (int x, int y);

/**
 * @brief Draw the ghosts and bullets on LCD
 * Draw the ghosts animation 
 * Draws the bullets as a filled circle.
*/
void Ghost_Bullet_Draw(void);

extern Ghost_t ghosts[GHOST_MAX];
extern int ghost_count;
extern int life;
extern int shield;
extern void hit_melody (void);

#endif