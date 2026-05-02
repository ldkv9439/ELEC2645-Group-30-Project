/**
 * @file level.h
 * @brief Simple game level implementation 
 * Controls the amount of coins and ghosts added on each level
 */

#ifndef LEVEL_H
#define LEVEL_H

#include "level.h"
#include "stm32l4xx_hal.h"
#include "coins.h"
#include "ghost.h"

#include <stdint.h>

void Level1(void);
void Level2(void);
void Level3(void);
void BossLevel(void);

/**
 * @enum LevelState_t
 * @brief Set up states for each level
 */
typedef enum {
    LEVEL1 = 0,  
    LEVEL2,      
    LEVEL3,      
    BOSSLEVEL
} LevelState_t;

extern LevelState_t level_state;
extern uint32_t boss_level_start_time;


#endif