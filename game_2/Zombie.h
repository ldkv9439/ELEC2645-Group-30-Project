/**
 * @file Zombie.h
 * @brief Zombie objects for Plants vs Zombies
 */
#ifndef ZOMBIE_H
#define ZOMBIE_H

#include <stdint.h>
#include "Utils.h"

typedef enum {
    ZOMBIE_NORMAL = 0,
    ZOMBIE_CONE   = 1,
} ZombieType;

#define HP_ZOMBIE_NORMAL  200
#define HP_ZOMBIE_CONE    500

#define ZOMBIE_SPEED_DIV_NORMAL  2
#define ZOMBIE_SPEED_DIV_CONE    3

#define ZOMBIE_BITE_DMG   2

#define SCORE_ZOMBIE_NORMAL  10
#define SCORE_ZOMBIE_CONE    25

/* Scale for zombie sprites (16x20 -> 32x40) */
#define ZOMBIE_SCALE  2

typedef struct {
    ZombieType type;
    uint8_t      active;
    int16_t      x;
    int16_t      y;
    int16_t      lane;
    int16_t      hp;
    int16_t      hp_max;
    uint8_t      move_timer;
    uint8_t      eating;
} Zombie;

void     Zombie_Init(Zombie* zombie, ZombieType type, int16_t lane);
void     Zombie_Update(Zombie* zombie);
void     Zombie_Draw(Zombie* zombie);
void     Zombie_TakeDamage(Zombie* zombie, int16_t dmg);
AABB     Zombie_GetAABB(Zombie* zombie);
uint16_t Zombie_GetScore(Zombie* zombie);

#endif /* ZOMBIE_H */
