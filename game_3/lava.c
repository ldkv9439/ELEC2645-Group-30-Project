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

// ===== ANIMATION SPRITES =====
/**
 * @brief IDLE first animation - fire stays static 
 * 8x8 pixel sprite showing idle
 */
const uint8_t LavaIDLE1[8][8] = {
    {255,255,2,255,2,255,255,255},
    {255,2,255,255,2,2,255,2},
    {255,255,255,2,2,2,255,255},
    {255,255,2,2,5,2,2,255},
    {255,2,2,5,5,2,2,255},
    {255,2,5,6,6,2,2,255},
    {255,2,5,6,5,2,255,255},
    {255,255,2,6,2,255,255,255}
};

const uint8_t LavaIDLE2[8][8] = {
    {255,2,255,2,255,255,255,255},
    {255,255,255,2,2,255,2,255},
    {2,255,2,2,2,2,255,255},
    {255,255,2,2,5,2,255,255},
    {255,2,2,5,5,2,2,255},
    {255,2,5,6,6,2,2,255},
    {255,2,5,6,5,2,255,255},
    {255,255,2,6,2,255,255,255}
};

static const uint8_t* LAVA_FRAMES[LAVA_FRAME_COUNT] = {
    (const uint8_t*)LavaIDLE1,
    (const uint8_t*)LavaIDLE2
};

const Lava_Coordinates_t lava_set [LAVA_MAX] = {
    {80,170}, {130,75}, {180,130}, {30,120}
};

Lava_t lava [LAVA_MAX];
static uint32_t last_hit_time = 0;

void lava_hit_melody (void) {
    buzzer_note(&buzzer_cfg, NOTE_A4, 40);
    HAL_Delay(60);

    buzzer_note(&buzzer_cfg, NOTE_G4, 40);
    HAL_Delay(60);

    buzzer_note(&buzzer_cfg, NOTE_F4, 40);
    HAL_Delay(100);

    buzzer_off(&buzzer_cfg);
}

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
    uint32_t current_time = HAL_GetTick();

    for (int i = 0; i < LAVA_MAX; i++) {
        if(lava[i].active && game_character.dash_counter == 0) {
            if(Circle_Overlap(character->x, character->y, 16, lava[i].x, lava[i].y, LAVA_RADIUS)) {

                if (current_time - last_hit_time >= 1000) {
                    if (shield > 0 ) {
                        shield--; 
                    } else {
                        life--;
                    }

                    lava_hit_melody();
                    last_hit_time = current_time;
                }
            }
        }
    }
}

void Lava_Draw (void) {

    // Set up ghost animation
    static uint16_t frame_counter = 0;
    static uint32_t animation_frame = 0;

    frame_counter++;

    if (frame_counter > 10){
        frame_counter = 0;
        animation_frame = (animation_frame + 1) % 2;
    }

    // Draw lava as a filled rectangle
    // Color: Red (2 in 4-bit color), filled (1)
    for (int i =0; i < LAVA_MAX; i++) {
        if(lava[i].active) {
            LCD_Draw_Sprite_Scaled(lava[i].x, lava[i].y, 8, 8, (uint8_t*)LAVA_FRAMES[animation_frame], 3);
        }
    }
}