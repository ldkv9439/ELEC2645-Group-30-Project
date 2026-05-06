/**
 * @file coins.c
 * @brief Coin object implementation 
 */

#include "Coins.h"
#include "coins.h"
#include "stm32l4xx_hal.h"
#include "Buzzer.h"
#include "PWM.h"
#include "LCD.h"
#include "Level.h"
#include "Ghost.h"
#include "Flame.h"
#include "Character.h"

#include <stdint.h>
#include <stdlib.h>

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg_2;

static Coins_t coins [COINS_MAX];

int coins_remaining = 0;
uint16_t score = 0;

// Function to detect collision between two circles based on its radius
uint8_t Circle_Overlap(uint16_t x1, uint16_t y1, uint16_t r1,
                        uint16_t x2, uint16_t y2, uint16_t r2)
{
    int32_t dx = (int32_t)x2 - (int32_t)x1;
    int32_t dy = (int32_t)y2 - (int32_t)y1;
    int32_t dist_squared = (dx * dx) + (dy * dy);
    int32_t radii_sum = r1 + r2;
    int32_t radii_sum_squared = radii_sum * radii_sum;
    
    return (dist_squared <= radii_sum_squared) ? 1 : 0;
}

// Buzzer sound when character collects coins
void coins_melody (void) {
    buzzer_note(&buzzer_cfg, NOTE_G7, 40);
    HAL_Delay(20);

    buzzer_note(&buzzer_cfg, NOTE_C8, 40);
    HAL_Delay(20);
    
    buzzer_off(&buzzer_cfg);
}

void Coins_Reset (void) {
    coins_remaining = 0;

    for (int i = 0; i < COINS_MAX; i++) {
        coins[i].collect = 1;
    }
}

void Coins_Add (int amount) {
    // Add coins based on the amount set in each level in random position within boundaries
    coins_remaining = amount;
    
    int ghost_radius = NORMAL_GHOST_ADD_RADIUS;
    int ghost_half = NORMAL_GHOST_HALF;

    if (level_state == BOSSLEVEL) {
        ghost_radius = BOSS_GHOST_ADD_RADIUS;
        ghost_half = BOSS_GHOST_HALF;
    }

    for (int i = 0; i < amount; i++) {

        uint8_t coins_placed = 0;
        int tries = 0;
        int x = COINS_MIN_X + (rand () % (COINS_MAX_X - COINS_MIN_X));
        int y = COINS_MIN_Y + (rand () % (COINS_MAX_Y - COINS_MIN_Y));

        // Check collision between spawned coins with other coins, ghost, flame and character sprite
        while(!coins_placed && tries < 100) {
            x = COINS_MIN_X + (rand () % (COINS_MAX_X - COINS_MIN_X));
            y = COINS_MIN_Y + (rand () % (COINS_MAX_Y - COINS_MIN_Y));

            int collision = 0;

            // Collision between coins
            for (int j = 0; j < i; j++) {
                if (Circle_Overlap(x, y, COINS_RADIUS, coins[j].x, coins[j].y, COINS_SPACING)) {
                    collision = 1;
                    break;
                }
            }

            // Collision with ghosts
            for (int g = 0; g < ghost_count; g++) {

                int ghost_center_x = ghosts[g].x + ghost_half;
                int ghost_center_y = ghosts[g].y + ghost_half;

                if (Circle_Overlap(x, y, COINS_RADIUS, ghost_center_x, ghost_center_y, ghost_radius)) {
                    collision = 1;
                    break;
                }
            }

            // Collision with flame
            for (int h = 0; h < FLAME_MAX; h++) {

                if (!flame[h].active) continue;

                int flame_center_x = flame[h].x + FLAME_SIZE_HALF;
                int flame_center_y = flame[h].y + FLAME_SIZE_HALF;

                if (Circle_Overlap(x, y, COINS_RADIUS, flame_center_x, flame_center_y, FLAME_ADD_RADIUS)) {
                    collision = 1;
                    break;
                }
            }

            // Collision with character
            if (Circle_Overlap(x, y, COINS_RADIUS, game_character.x, game_character.y, CHAR_HALF)) {
                collision = 1;
            }

            // Place coins if there is no overlapping with other objects
            if (!collision) {
                coins_placed = 1;
            }

            tries++;
        }

        coins[i].x = x;
        coins[i].y = y;
        coins[i].collect = 0;
    }
}


void Coins_Update (Character_t* character) {
    // Update number of coins remaining and the curent score
    static uint8_t led_on = 0;
    static uint32_t current_time = 0;

    for (int i = 0; i < COINS_MAX; i++) {
        // Check collision between the character and coins
        if(!coins[i].collect && Circle_Overlap(character->x, character->y, CHAR_HALF, coins[i].x, coins[i].y, COINS_RADIUS)) {
            coins[i].collect = 1;
            coins_remaining--;

            // Only play the buzzer sound when collecting coins if not in dashing mode
            if (game_character.dash_counter == 0){
                coins_melody();
            }

            led_on = 1; 
            current_time = HAL_GetTick();
            PWM_SetDuty(&pwm_cfg_2, 0); // Turn off yellow LED
            
            score++;
            LCD_Draw_Rect(35, 17, 40, 16, 0, 1); // Clear the score board
        }

        // Make the yellow LED blink 
        if (led_on == 1){
            if (HAL_GetTick() - current_time > 100){
                led_on = 0;
                PWM_SetDuty(&pwm_cfg_2, 100);   // Turn off yellow LED
            }
        }
    }
}

void Coins_Draw (void) {
    // Draw coins as a filled circle
    // Color: gold (10 in 4-bit color), filled (1)
    for (int i =0; i < COINS_MAX; i++) {
        if(!coins[i].collect) {
            LCD_Draw_Circle(coins[i].x, coins[i].y, COINS_RADIUS, 10, 1);
        }
    }
}
