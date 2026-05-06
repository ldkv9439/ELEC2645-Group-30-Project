/**
 * @file Flame.c
 * @brief Flame object implementation 
 */

#include "Flame.h"
#include "stm32l4xx_hal.h"
#include "Buzzer.h"
#include "PWM.h"
#include "LCD.h"
#include "Level.h"
#include "Coins.h"
#include "Character.h"
#include "Ghost.h"
#include "Sprites.h"

#include <stdint.h>

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg;

static uint32_t last_hit_time = 0;

static const uint8_t* FLAME_FRAMES[FLAME_FRAME_COUNT] = {
    (const uint8_t*)FlameIDLE1,
    (const uint8_t*)FlameIDLE2
};

Flame_t flame [FLAME_MAX];

// Manage flame spawning positions
const Flame_Coordinates_t flame_set [FLAME_MAX] = {
    {80,170}, {130,75}, {180,130}, {30,120}
};

// Buzzer sound when character hits flame
void flame_hit_melody (void) {
    buzzer_note(&buzzer_cfg, NOTE_A4, 40);
    HAL_Delay(80);

    buzzer_note(&buzzer_cfg, NOTE_G4, 40);
    HAL_Delay(80);

    buzzer_note(&buzzer_cfg, NOTE_F4, 40);
    HAL_Delay(150);

    buzzer_off(&buzzer_cfg);
}

void Flame_Add (int amount) {
    // Add flame based on the amount set in each level
    for (int i = 0; i < amount; i++) {
        flame[i].active = 1;
        flame[i].x = flame_set[i].x;
        flame[i].y = flame_set[i].y;
    }
}

void Flame_Reset (void) {
    for (int i = 0; i < FLAME_MAX; i++) {
        flame[i].active = 0;
    }
}

void Flame_Update (Character_t *character) {
    uint32_t current_time = HAL_GetTick();

    // Handle collision betwen the character and the flame 
    for (int i = 0; i < FLAME_MAX; i++) {

        int flame_center_x = flame[i].x + FLAME_SIZE_HALF;
        int flame_center_y = flame[i].y + FLAME_SIZE_HALF;

        if(flame[i].active && game_character.dash_counter == 0) {
            if(Circle_Overlap(character->x, character->y, CHAR_COLLISION_RADIUS, flame_center_x, flame_center_y, FLAME_COLLISION_RADIUS)) {

                if (current_time - last_hit_time >= 1000) {
                    if (shield > 0 ) {
                        shield--; 
                    } else {
                        life--;
                    }

                    flame_hit_melody();
                    last_hit_time = current_time;
                }
            }
        }
    }
}

void Flame_Draw (void) {

    // Set up flame animation
    static uint16_t frame_counter = 0;
    static uint32_t animation_frame = 0;

    frame_counter++;

    if (frame_counter > 10){
        frame_counter = 0;
        animation_frame = (animation_frame + 1) % 2;
    }

    // Draw ghost animation
    for (int i =0; i < FLAME_MAX; i++) {
        if(flame[i].active) {
            LCD_Draw_Sprite_Scaled(flame[i].x, flame[i].y, 8, 8, (uint8_t*)FLAME_FRAMES[animation_frame], 3);
        }
    }
}