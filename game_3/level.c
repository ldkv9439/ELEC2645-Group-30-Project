/**
 * @file level.c
 * @brief Game level implementation 
 */

#include "level.h"
#include "stm32l4xx_hal.h"
#include "coins.h"
#include "ghost.h"
#include "lava.h"

#include <stdint.h>

int status_level = 0;

/**
 * @brief Get level state name
 */
const char* get_level_state(LevelState_t state) {
    switch (state) {
        case LEVEL1: return "LEVEL1";
        case LEVEL2:    return "LEVEL2";
        case LEVEL3:    return "LEVEL3";
        case BOSSLEVEL: return "BOSS";
        default:          return "???";
    }
}

void Level1 (void) {
    Ghost_Reset();
    Ghost_Add(20,190);
    Ghost_Add(60,60);
    Ghost_Add(180,185);
    Ghost_Add(190,80);
    Coins_Reset();
    Coins_Add(5);
    Lava_Reset();

    status_level = 1;
}

void Level2 (void) {
    Ghost_Reset();
    Ghost_Add(20,190);
    Ghost_Add(60,60);
    Ghost_Add(180,185);
    Ghost_Add(190,90);
    Coins_Reset();
    Coins_Add(7);
    Lava_Reset();
    Lava_Add(3);

    status_level = 1;
}

void Level3 (void) {
    Ghost_Reset();
    Ghost_Add(20,190);
    Ghost_Add(60,60);
    Ghost_Add(180,185);
    Ghost_Add(190,90);
    Ghost_Add(120,130);
    Coins_Reset();
    Coins_Add(10);
    Lava_Reset();
    Lava_Add(4);

    status_level = 1;
}

void BossLevel (void) {
    Ghost_Reset();
    Ghost_Add (80,100);
    Coins_Reset();
    Coins_Add(5);
    Lava_Reset();

    status_level = 1;
}