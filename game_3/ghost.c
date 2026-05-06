/**
 * @file ghost.c
 * @brief Ghost object implementation 
 */

#include "Ghost.h"
#include "coins.h"
#include "ghost.h"
#include "stm32l4xx_hal.h"
#include "Buzzer.h"
#include "LCD.h"
#include "Level.h"
#include "Coins.h"
#include "Character.h"
#include "Sprites.h"

#include <stdint.h>
#include <stdlib.h>

extern Buzzer_cfg_t buzzer_cfg;

static Bullet_t bullets[GHOST_MAX_BULLETS];
static uint32_t last_shot_time = 0;
static uint32_t shoot_interval_time = 1500; // 1500 milliseconds

static const uint8_t* GHOST_FRAMES[GHOST_FRAME_COUNT] = {
    (const uint8_t*)GhostIDLE1,
    (const uint8_t*)GhostIDLE2
};

Ghost_t ghosts[GHOST_MAX];

int ghost_count = 0;
int life = LIFE_MAX;
int shield = SHIELD_MAX;
int shield_recharge = 0;

// Buzzer sound when character gets hit by a bullet
void bullet_hit_melody (void) {
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
    if (current_time - last_shot_time >= shoot_interval_time) {

        for (int i = 0; i < ghost_count; i++) {
            Bullet_Update(ghosts[i].x, ghosts[i].y);
        } 

        last_shot_time = current_time;
    }

    // Applying boundaries on the area bullets can move depending on levels
    // Customize speed of bullets depending on levels
    for (int b = 0; b < GHOST_MAX_BULLETS; b++) {

        int shoot_speed = NORMAL_BULLET_SPEED;
        int x_min = NORMAL_X_MIN;
        int y_min = NORMAL_Y_MIN;
        int x_max = NORMAL_X_MAX;
        int y_max = NORMAL_Y_MAX;

        if (level_state == BOSSLEVEL) {
            shoot_speed = BOSS_BULLET_SPEED;
            x_min = BOSS_X_MIN;
            x_max = BOSS_X_MAX;
            y_max = BOSS_Y_MAX;
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

    // Handle collision between the character and the bullets 
    for (int j = 0; j < GHOST_MAX_BULLETS; j++) {

        int bullet_radius = NORMAL_BULLET_RADIUS;

        if (level_state == BOSSLEVEL) {
            bullet_radius = BOSS_BULLET_RADIUS;
        }

        // Character will not get any impact if collision happens when its in dashing mode
        if (bullets[j].valid == 1 && game_character.dash_counter == 0){
            // Character shield is deducted first before deducting character life
            if (Circle_Overlap(character->x, character->y, CHAR_HALF, bullets[j].x, bullets[j].y, bullet_radius)) {
                if (shield > 0) {
                    if (level_state == BOSSLEVEL) {
                        shield -= 2;
                    } else {
                        shield--;
                    }
                } else {
                    life--;
                }

            bullets[j].valid = 0;   // Clear flag

            LCD_Draw_Rect(155, 17, 40, 16, 0, 1); // clear the score board
            LCD_Draw_Rect(30, 34, 40, 16, 0, 1); // clear the shield count

            bullet_hit_melody();

            }
        }
    }

    // Recharge and Charge Shield every 50 miliseconds 
    if (shield < SHIELD_MAX) {
        shield_recharge++;
    } if (shield_recharge > 50) {
        shield++;
        shield_recharge = 0;
    }

    if (level_state == BOSSLEVEL) {
        for (int i = 0; i < ghost_count; i++) {
            int dx = (game_character.x + CHAR_HALF) - (ghosts[i].x + BOSS_GHOST_HALF);
            int dy = (game_character.y + CHAR_HALF) - (ghosts[i].y + BOSS_GHOST_HALF);

            int boss_ghost_speed = BOSS_GHOST_SPEED;

            if (dx > 0) ghosts[i].x += boss_ghost_speed;
            else if (dx < 0) ghosts[i].x -= boss_ghost_speed;

            if (dy > 0) ghosts[i].y += boss_ghost_speed;
            else if (dy < 0) ghosts[i].y -= boss_ghost_speed;

            if (ghosts[i].x < 10) ghosts[i].x = 10;
            if (ghosts[i].x > 169) ghosts[i].x = 169;
            if (ghosts[i].y < 55) ghosts[i].y = 55;
            if (ghosts[i].y > 169) ghosts[i].y = 169;
        }
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
    int ghost_scale = NORMAL_GHOST_SCALE;
    int bullet_radius = NORMAL_BULLET_RADIUS;

    if (level_state == BOSSLEVEL) {
        ghost_scale = BOSS_GHOST_SCALE;
        bullet_radius = BOSS_BULLET_RADIUS;
    }
    
    // Draw ghost animation
    for (int i = 0; i < ghost_count; i++) {
        LCD_Draw_Sprite_Scaled(ghosts[i].x, ghosts[i].y, 8, 8, (uint8_t*)GHOST_FRAMES[animation_frame], ghost_scale);
    }

    // Draw bullets
    for (int i = 0; i < GHOST_MAX_BULLETS; i++) {
        if (bullets[i].valid) {
            LCD_Draw_Circle(bullets[i].x, bullets[i].y, bullet_radius, 1, 1);

            if (bullets[i].x < 10) bullets[i].x = 10;
            if (bullets[i].x > 225) bullets[i].x = 225;
            if (bullets[i].y < 60) bullets[i].y = 60;
            if (bullets[i].y > 225) bullets[i].y = 225;
        }
    }
}