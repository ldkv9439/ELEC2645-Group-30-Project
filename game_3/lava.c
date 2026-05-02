/**
 * @file lava.c
 * @brief Lava object implementation 
 */

#include "lava.h"
#include "coins.h"
#include "ghost.h"
#include "PWM.h"
#include "level.h"
#include "stm32l4xx_hal.h"
#include "tim.h"
#include "Buzzer.h"
#include "LCD.h"
#include "Character.h"

#include <stdint.h>
#include <stdlib.h>

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg;

const Lava_Coordinates_t lava_set [LAVA_MAX] = {
    {30,80},  {90,170}, {120,85}, {180,105}, {150,175}
};

Lava_t lava [LAVA_MAX];
static uint32_t last_hit_time = 0;

void Lava_Add (int amount) {
    // Add ghosts based on the amount set in each level
    for (int i = 0; i < amount; i++) {
        lava[i].active = 1;
        lava[i].x = lava_set[i].x;
        lava[i].y = lava_set[i].y;
    }
}

void Lava_Reset (void) {
    for (int i = 0; i < LAVA_MAX; i++) {
        lava[i].active = 0;
    }
}

void Lava_Update (Character_t *character) {
    static uint32_t current_time = 0;

    for (int i = 0; i < LAVA_MAX; i++) {
        if(lava[i].active && Circle_Overlap(character->x, character->y, 16, lava[i].x, lava[i].y, LAVA_RADIUS)) {

            current_time = HAL_GetTick();

            if (current_time - last_hit_time >= 1000) {
                if (shield > 0 ) {
                    shield--; 
                } else {
                    life--;
                }

                last_hit_time = current_time;
            }
        }
    }
}

void Lava_Draw (void) {
    // Draw lava as a filled rectangle
    // Color: Red (2 in 4-bit color), filled (1)
    for (int i =0; i < LAVA_MAX; i++) {
        if(lava[i].active) {
            LCD_Draw_Rect(lava[i].x, lava[i].y, LAVA_SIZE, LAVA_SIZE, 15, 1);
        }
    }
}