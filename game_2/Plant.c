/**
 * @file Plant.c
 * @brief Implements plant behaviour, rendering, and special abilities for PvZ gameplay.
 *
 * Handles:
 * - Initialisation of plant properties (type, position, health, timers)
 * - Per-frame updates (shooting, sun generation, cherry bomb explosion timing)
 * - Sprite rendering with 2x scaling and visual effects
 * - Damage handling and collision bounding boxes
 *
 * Special behaviour:
 * - Peashooters trigger projectile events
 * - Sunflowers generate sun over time
 * - Cherry bombs detonate after a delay and remove themselves after exploding
 */
#include "Plant.h"
#include "LCD.h"
#include "Sprites.h"

/* Scaled sprite size in pixels */
#define SPR_W  (16 * PLANT_SCALE)   /* 32 */
#define SPR_H  (16 * PLANT_SCALE)   /* 32 */

void Plant_Init(Plant* plant, PlantType type, int16_t col, int16_t row) {
    plant->type      = type;
    plant->active    = 1;
    plant->grid_col  = col;
    plant->grid_row  = row;
    plant->shoot_ready = 0;
    plant->sun_ready   = 0;
    plant->explode_ready = 0;
    plant->explode_timer = 0;

    /* Centre sprite horizontally in cell, top-align vertically */
    plant->x = GRID_ORIGIN_X + col * CELL_W + (CELL_W - SPR_W) / 2;
    plant->y = GRID_ORIGIN_Y + row * CELL_H + (CELL_H - SPR_H) / 2;
    

    switch (type) {
        case PLANT_PEASHOOTER: plant->hp_max = HP_PEASHOOTER; plant->timer = SHOOT_INTERVAL_PEASHOOTER; break;
        case PLANT_A_PEASHOOTER: plant->hp_max = HP_A_PEASHOOTER; plant->timer = SHOOT_INTERVAL_A_PEASHOOTER; break;
        case PLANT_SUNFLOWER:  plant->hp_max = HP_SUNFLOWER;  plant->timer = SUN_INTERVAL_SUNFLOWER;    break;
        case PLANT_WALLNUT:    plant->hp_max = HP_WALLNUT;    plant->timer = 0;                         break;
        case PLANT_CHERRY_BOMB: plant->hp_max = HP_CHERRY_BOMB; plant->timer = EXPLODE_DURATION_CHERRY_BOMB;break;
        default:              plant->hp_max = 0;           plant->timer = 0;                         break;
    }
    plant->hp = plant->hp_max;
}

void Plant_Update(Plant* plant) {
    if (!plant->active) return;
    plant->shoot_ready = 0;
    plant->sun_ready   = 0;

    if (plant->explode_timer > 0)
    {plant->explode_timer--;}

    if (plant->timer > 0) {
        plant->timer--;
    } else {
        if (plant->type == PLANT_PEASHOOTER) {
            plant->shoot_ready = 1;
            plant->timer = SHOOT_INTERVAL_PEASHOOTER;
        } else if (plant->type == PLANT_A_PEASHOOTER) {
            plant->shoot_ready = 1;
            plant->timer = SHOOT_INTERVAL_A_PEASHOOTER;
        }
        else if (plant->type == PLANT_SUNFLOWER) {
            plant->sun_ready = 1;
            plant->timer = SUN_INTERVAL_SUNFLOWER;
        }
        else if (plant->type == PLANT_CHERRY_BOMB) {
            if(!plant->explode_ready) {
            plant->explode_ready = 1;
            plant->explode_timer = 15;}
        }
    }
}

void Plant_Draw(Plant* plant) {

    // if plant is not active, don't draw anything
    if (!plant->active) return;

    // draw sprite based on type
    switch (plant->type) {
        case PLANT_PEASHOOTER:                             // normal peashooter
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                PEASHOOTER_ROWS, PEASHOOTER_COLS,
                (const uint8_t*)SPRITE_PEASHOOTER, PLANT_SCALE);
            break;
        case PLANT_A_PEASHOOTER:                        // advance peashooter
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                A_PEASHOOTER_ROWS, A_PEASHOOTER_COLS,
                (const uint8_t*)A_SPRITE_PEASHOOTER, PLANT_SCALE);
            break;
        case PLANT_SUNFLOWER:                              // sunflower
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                SUNFLOWER_ROWS, SUNFLOWER_COLS,
                (const uint8_t*)SPRITE_SUNFLOWER, PLANT_SCALE);
            break;
        case PLANT_WALLNUT:                                // wallnut
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                WALLNUT_ROWS, WALLNUT_COLS,
                (const uint8_t*)SPRITE_WALLNUT, PLANT_SCALE);
            break;
        case PLANT_CHERRY_BOMB:                           // cherry bomb
            if (plant->explode_timer > 0) {
            // show explosion
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                EXPLOSION_ROWS, EXPLOSION_COLS,
                (const uint8_t*)SPRITE_EXPLOSION, PLANT_SCALE);
                plant->active = 0;  // cherry bomb disappears immediately when exploding
            } 
            else {
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                CHERRY_ROWS, CHERRY_COLS,
                (const uint8_t*)SPRITE_CHERRY, PLANT_SCALE);
                {
                // flash red overlay in last 1 second
                if (plant->timer < 20 && (plant->timer % 4 < 2)) {
                    LCD_Draw_Rect(plant->x, plant->y, SPR_W, SPR_H, 3, 0);
                }
            }}
            break;
        default: break;
    }

    /* HP bar below sprite, only when damaged */
    if (plant->hp < plant->hp_max) {
        int16_t bar_w  = SPR_W;
        int16_t filled = (int16_t)((int32_t)bar_w * plant->hp / plant->hp_max);
        int16_t bar_y  = plant->y + SPR_H + 1;
        LCD_Draw_Rect(plant->x, bar_y, bar_w, 3, 2, 1);
        if (filled > 0)
            LCD_Draw_Rect(plant->x, bar_y, filled, 3, 3, 1);
    }
}

void Plant_TakeDamage(Plant* plant, int16_t dmg) {
    if (!plant->active) return;
    plant->hp -= dmg;
    if (plant->hp <= 0) { plant->hp = 0; plant->active = 0; }
}

AABB Plant_GetAABB(Plant* plant) {
    AABB box;
    box.x      = plant->x;
    box.y      = plant->y;
    box.width  = SPR_W;
    box.height = SPR_H;
    return box;
}
