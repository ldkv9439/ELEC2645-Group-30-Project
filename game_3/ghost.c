/**
 * @file ghost.c
 * @brief Ghost object implementation 
 */

#include "ghost.h"
#include "stm32l4xx_hal.h"
#include "tim.h"  
#include "Buzzer.h"
#include "Character.h"
#include "level.h"

#include <stdint.h>
#include <stdlib.h>

extern Buzzer_cfg_t buzzer_cfg;

Ghost_t ghosts[GHOST_MAX];
Bullet_t bullets[GHOST_MAX_BULLETS];

static uint32_t last_shot_time = 0;
static uint32_t shoot_interval_time = 1500; // 1500 milliseconds

int ghost_count = 0;
int life = LIFE_MAX;
char life_str[20];
int shield = SHIELD_MAX;
char shield_str[20];
int shield_recharge = 0;

// ===== ANIMATION SPRITES =====
/**
 * @brief IDLE first animation - ghost stays static 
 * 8x8 pixel sprite showing idle
 */
const uint8_t GhostIDLE1[8][8] = {
    {255,255,13,13,13,1,255,255},
    {255,13,13,13,1,1,1,255},
    {255,13,13,1,1,1,1,1},
    {13,13,255,255,1,255,255,1},
    {13,13,255,2,1,255,2,1},
    {13,13,13,1,1,1,1,1},
    {13,13,13,1,1,1,1,1},
    {13,13,255,1,1,255,1,1}
};

/**
 * @brief IDLE second animation - ghost stays static but legs pattern changed
 * 8x8 pixel sprite showing idle
 */
const uint8_t GhostIDLE2[8][8] = {
    {255,255,13,13,13,1,255,255},
    {255,13,13,13,1,1,1,255},
    {255,13,13,1,1,1,1,1},
    {13,13,255,255,1,255,255,1},
    {13,13,255,2,1,255,2,1},
    {13,13,13,1,1,1,1,1},
    {13,13,13,1,1,1,1,1},
    {13,255,1,1,255,1,1,255}
};

static const uint8_t* GHOST_FRAMES[GHOST_FRAME_COUNT] = {
    (const uint8_t*)GhostIDLE1,
    (const uint8_t*)GhostIDLE2
};

void hit_melody (void) {
    buzzer_note(&buzzer_cfg, NOTE_DS5, 40);
    HAL_Delay(15);

    buzzer_note(&buzzer_cfg, NOTE_CS5, 40);
    HAL_Delay(15);

    buzzer_note(&buzzer_cfg, NOTE_C4, 40);
    HAL_Delay(15);

    buzzer_off(&buzzer_cfg);
}

void Ghost_Add(int x, int y) {
    // Add ghosts based on the amount set in each level
    if (ghost_count < GHOST_MAX) {
        ghosts[ghost_count].x = x;
        ghosts[ghost_count].y = y;
        ghost_count++;
    }
}

void Ghost_Reset (void) {
    ghost_count = 0;

    for (int i = 0; i < GHOST_MAX_BULLETS; i++) {
        bullets[i].valid = 0;
    }
}

void Bullet_Update (int x, int y) {
    // Update direction of bullets
    const int direction_dx []= {0, 0, 1, 1, 1, 0, -1, -1, -1};
    const int direction_dy []= {0, -1,-1, 0, 1, 1, 1, 0,-1};

    int d = (rand() % 8) + 1;   // Choose a random direction each time

    for (int i = 0; i < GHOST_MAX_BULLETS; i++) {
        if (!bullets[i].valid) {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].dx = direction_dx[d];
            bullets[i].dy = direction_dy[d];
            bullets[i].valid = 1;
            break;
        }
    }
}

void Ghost_Update (Character_t* character) {
    // Update each ghost to shoot the bullets
    uint32_t current_time = HAL_GetTick();

    // Shoot bullets every 1500 milliseconds
    if (current_time - last_shot_time >= shoot_interval_time){
        for (int i = 0; i < ghost_count; i++) {
            Bullet_Update(ghosts[i].x, ghosts[i].y);
        } 
        last_shot_time = current_time;
    }

    // Applying boundaries on the area bullets can move depending on levels
    // Customize speed of bullets depending on levels
    for (int b = 0; b < GHOST_MAX_BULLETS; b++) {

        int shoot_speed = 2;
        int x_min = 10;
        int y_min = 60;
        int x_max = 225;
        int y_max = 225;

        if (level_state == BOSSLEVEL) {
            shoot_speed = 5;
            x_min = 25;
            x_max = 200;
            y_max = 200;
        }

        // Make the bullets bounce opposite direction (x-direction / y-direction) when hit the game wall on LCD
        if (bullets[b].valid) {
            bullets[b].x += bullets[b].dx *shoot_speed;
            bullets[b].y += bullets[b].dy *shoot_speed;

            if (bullets[b].x <= x_min) {
                bullets[b].x = x_min;
                bullets[b].dx = -bullets[b].dx;
            }

            if (bullets[b].x >= x_max) {
                bullets[b].x = x_max;
                bullets[b].dx = -bullets[b].dx;
            }

            if (bullets[b].y <= y_min)  {
                bullets[b].y = y_min;
                bullets[b].dy = -bullets[b].dy;
            }

            if (bullets[b].y >= y_max) {
                bullets[b].y = y_max;
                bullets[b].dy = -bullets[b].dy;
            }
        }
    }

    // Handle collision betwen the character and the bullets 
    for (int j = 0; j < GHOST_MAX_BULLETS; j++) {
        // Character will not get any impact if collision happens when its in dashing mode
        if (bullets[j].valid == 1 && game_character.dash_counter == 0){
            // Character shield is deducted first before deducting character life
            if (Circle_Overlap(character->x, character->y, 16, bullets[j].x, bullets[j].y, 3)) {
                if (shield > 0) {
                    shield--;
                } else {
                    life--;
                }

            bullets[j].valid = 0;   // Clear flag

            LCD_Draw_Rect(155, 17, 40, 16, 0, 1); // clear the score board
            LCD_Draw_Rect(30, 34, 40, 16, 0, 1); // clear the shield count

            hit_melody();

            }
        }
    }

    // Recharge and Charge Shield every 50 miliseconds 
    if (shield < 5) {
        shield_recharge++;
    } if (shield_recharge > 50) {
        shield++;
        shield_recharge = 0;
    }

}

void Ghost_Bullet_Draw (void) {
    // Set up ghost animation
    static uint16_t frame_counter = 0;
    static uint32_t animation_frame = 0;

    frame_counter++;

    if (frame_counter > 10){
        frame_counter = 0;
        animation_frame = (animation_frame + 1) % 2;
    }

    // Set up different scale for ghostand bullets depending on levels
    int ghost_scale = 3;
    int bullet_scale = 5;

    if (level_state == BOSSLEVEL) {
        ghost_scale = 8;
        bullet_scale = 15;
    }
    
    // Draw ghost animation
    for (int i = 0; i < ghost_count; i++) {
        LCD_Draw_Sprite_Scaled(ghosts[i].x, ghosts[i].y, 8, 8, (uint8_t*)GHOST_FRAMES[animation_frame], ghost_scale);
    }

    // Draw bullets
    for (int i = 0; i < GHOST_MAX_BULLETS; i++) {
        if (bullets[i].valid) {
            LCD_Draw_Rect(bullets[i].x, bullets[i].y, bullet_scale, bullet_scale, 1, 1);
            if (bullets[i].x < 10) bullets[i].x = 10;
            if (bullets[i].x > 225) bullets[i].x = 225;
            if (bullets[i].y < 60) bullets[i].y = 60;
            if (bullets[i].y > 225) bullets[i].y = 225;
        }
    }
}