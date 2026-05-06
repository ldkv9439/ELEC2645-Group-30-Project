/**
 * @file ghost.h
 * @brief Simple ghost sprite for demo purposes
 * Controls the ghosts and bullets position and rendering.
 */
 
#ifndef GHOST_H
#define GHOST_H

#include "Character.h"

#include <stdint.h>

// ===== CONSTANTS =====

// ---- GHOSTS ----

// ---- Boundaries ----
#define NORMAL_X_MIN 10         // Normal levels minimum x-axis boundary
#define NORMAL_X_MAX 225        // Normal levels maximum x-axis boundary
#define NORMAL_Y_MIN 60         // Normal levels minimum y-axis boundary
#define NORMAL_Y_MAX 225        // Normal levels maximum y-axis boundary

#define BOSS_X_MIN 25           // Boss level minimum x-axis boundary
#define BOSS_X_MAX 200          // Boss level maximum x-axis boundary
#define BOSS_Y_MAX 200          // Boss level maximum y-axis boundary

// ---- Animation ----
#define GHOST_FRAME_COUNT 2     // Ghost animation frames

// ---- Limits ----
#define GHOST_MAX 5             // Maximum number of ghosts
#define GHOST_MAX_BULLETS 10    // Maximum number of bullets
#define LIFE_MAX 7              // Maximum number of character's life
#define SHIELD_MAX 5            // Maximum number of character's shield

// ---- Size ----
#define NORMAL_GHOST_HALF 12    // Half of normal ghost size (8x8 scaled to 3)
#define BOSS_GHOST_HALF 32      // Half of boss ghost size (8x8 scaled to 8)

#define NORMAL_GHOST_SCALE 3    // Normal ghost scale (24x24)
#define BOSS_GHOST_SCALE 8      // Boss ghost scale (64x64)

// ---- Collision ----
#define NORMAL_GHOST_COLLISION_RADIUS 8 // Normal ghost radius used to check collision with character (-4 from original radius)
#define BOSS_GHOST_COLLISION_RADIUS 25  // Ghost ghost radius used to check collision with character (-7 from original radius)

// ---- Used when adding coins ----
#define NORMAL_GHOST_ADD_RADIUS 25      // Normal ghost radius used when spawning coins (+13 from original radius)
#define BOSS_GHOST_ADD_RADIUS 45        // Boss ghost radius used when spawning coins (+13 from original radius)

// ---- Speed ----
#define BOSS_GHOST_SPEED 2      // Pixels per frame (boss ghost)

// ---- BULLETS ----

// ---- Speed ----
#define NORMAL_BULLET_SPEED 2   // Pixels per frame (bullets from normal ghost)
#define BOSS_BULLET_SPEED 5     // Pixels per frame (bullets from boss ghost)

// ---- Size ----
#define NORMAL_BULLET_RADIUS 3  // Bullet radius from normal ghost
#define BOSS_BULLET_RADIUS 7    // Bullet radius from boss ghost

// ===== DATA STRUCTURES =====

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

// ===== FUNCTIONS =====

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