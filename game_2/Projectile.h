/**
 * @file Projectile.h
 * @brief Pea projectile objects for Plants vs Zombies
 *
 * Peas fly right-to-left from peashooter barrels and damage the first
 * zombie they hit. They are removed when they hit a zombie or leave screen.
 */

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdint.h>
#include "Utils.h"

#define PEA_DAMAGE   50
#define PEA_SPEED     4   /* pixels per frame */

/**
 * @struct Projectile_t
 * @brief Single pea projectile
 */
typedef struct {
    uint8_t  active;   ///< 1 = in flight
    int16_t  x;        ///< Pixel X (left edge)
    int16_t  y;        ///< Pixel Y (top edge)
    int16_t  lane;     ///< Row lane it travels in (for collision optimisation)
} Projectile_t;

/**
 * @brief Fire a new pea from a peashooter's position
 * @param proj Pointer to projectile slot
 * @param x    Starting X pixel
 * @param y    Starting Y pixel
 * @param lane Grid row lane
 */
void Projectile_Fire(Projectile_t* proj, int16_t x, int16_t y, int16_t lane);

/**
 * @brief Advance pea position; deactivate if off screen
 * @param proj Pointer to projectile
 */
void Projectile_Update(Projectile_t* proj);

/**
 * @brief Draw pea sprite
 * @param proj Pointer to projectile
 */
void Projectile_Draw(Projectile_t* proj);

/**
 * @brief Return AABB for collision detection
 * @param proj Pointer to projectile
 * @return AABB bounding box
 */
AABB Projectile_GetAABB(Projectile_t* proj);

#endif /* PROJECTILE_H */
