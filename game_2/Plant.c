/**
 * @file Plant.c
 * @brief Plant implementation - sprites drawn at 2x scale
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
    /* Centre sprite horizontally in cell, top-align vertically */
    plant->x = GRID_ORIGIN_X + col * CELL_W + (CELL_W - SPR_W) / 2;
    plant->y = GRID_ORIGIN_Y + row * CELL_H + (CELL_H - SPR_H) / 2;
    plant->shoot_ready = 0;
    plant->sun_ready   = 0;

    switch (type) {
        case PLANT_PEASHOOTER: plant->hp_max = HP_PEASHOOTER; plant->timer = SHOOT_INTERVAL_PEASHOOTER; break;
        case PLANT_SUNFLOWER:  plant->hp_max = HP_SUNFLOWER;  plant->timer = SUN_INTERVAL_SUNFLOWER;    break;
        case PLANT_WALLNUT:    plant->hp_max = HP_WALLNUT;    plant->timer = 0;                         break;
        default:               plant->hp_max = 100;           plant->timer = 0;                         break;
    }
    plant->hp = plant->hp_max;
}

void Plant_Update(Plant* plant) {
    if (!plant->active) return;
    plant->shoot_ready = 0;
    plant->sun_ready   = 0;

    if (plant->timer > 0) {
        plant->timer--;
    } else {
        if (plant->type == PLANT_PEASHOOTER) {
            plant->shoot_ready = 1;
            plant->timer = SHOOT_INTERVAL_PEASHOOTER;
        } else if (plant->type == PLANT_SUNFLOWER) {
            plant->sun_ready = 1;
            plant->timer = SUN_INTERVAL_SUNFLOWER;
        }
    }
}

void Plant_Draw(Plant* plant) {
    
    if (!plant->active) return;

    switch (plant->type) {
        case PLANT_PEASHOOTER:
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                PEASHOOTER_ROWS, PEASHOOTER_COLS,
                (const uint8_t*)SPRITE_PEASHOOTER, PLANT_SCALE);
            break;
        case PLANT_SUNFLOWER:
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                SUNFLOWER_ROWS, SUNFLOWER_COLS,
                (const uint8_t*)SPRITE_SUNFLOWER, PLANT_SCALE);
            break;
        case PLANT_WALLNUT:
            LCD_Draw_Sprite_Scaled(plant->x, plant->y,
                WALLNUT_ROWS, WALLNUT_COLS,
                (const uint8_t*)SPRITE_WALLNUT, PLANT_SCALE);
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
