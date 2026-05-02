/**
 * @file coins.c
 * @brief Coin object implementation 
 */

#include "coins.h"
#include "PWM.h"
#include "ghost.h"
#include "lava.h"
#include "level.h"
#include "tim.h"
#include "Buzzer.h"
#include "LCD.h"
#include "Character.h"

#include <stdint.h>
#include <stdlib.h>

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg_2;

Coins_t coins [COINS_MAX];
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
    // Add coins based on the amount set in each level
    coins_remaining = amount;

    for (int i = 0; i < amount; i++) {

        uint8_t coins_placed = 0;
        int tries = 0;
        int x = MIN_X + (rand () % (MAX_X - MIN_X));
        int y = MIN_Y + (rand () % (MAX_Y - MIN_Y));

        while(!coins_placed && tries < 100) {
            x = MIN_X + (rand () % (MAX_X - MIN_X));
            y = MIN_Y + (rand () % (MAX_Y - MIN_Y));

            int collision = 0;

            for (int j = 0; j < i; j++) {
                if (Circle_Overlap(x, y, COINS_SPACING, coins[j].x, coins[j].y, COINS_SPACING)) {
                    collision = 1;
                    break;
                }
            }

            for (int g = 0; g < ghost_count; g++) {
                if (Circle_Overlap(x, y, COINS_SPACING, ghosts[g].x, ghosts[g].y, COINS_SPACING)) {
                    collision = 1;
                    break;
                }
            }

            for (int h = 0; h < LAVA_MAX; h++) {
                if (Circle_Overlap(x, y, COINS_SPACING, lava[h].x, lava[h].y, COINS_SPACING)) {
                    collision = 1;
                    break;
                }
            }

            if (Circle_Overlap(x, y, COINS_SPACING, game_character.x, game_character.y, COINS_SPACING)) {
                collision = 1;
            }

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
        if(!coins[i].collect && Circle_Overlap(character->x, character->y, 16, coins[i].x, coins[i].y, COINS_RADIUS)) {
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
            LCD_Draw_Circle(coins[i].x, coins[i].y, 4, 10, 1);
        }
    }
}
