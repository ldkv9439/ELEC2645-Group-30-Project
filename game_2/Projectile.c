/**
 * @file Projectile.c
 * @brief Pea projectile - drawn at 2x scale
 */
#include "Projectile.h"
#include "LCD.h"
#include "Sprites.h"

#define SCREEN_WIDTH 240
#define PEA_SCALE    2

void Projectile_Fire(Projectile_t* proj, int16_t x, int16_t y, int16_t lane) {
    proj->active = 1;
    proj->x      = x;
    proj->y      = y;
    proj->lane   = lane;
}

void Projectile_Update(Projectile_t* proj) {
    if (!proj->active) return;
    proj->x += PEA_SPEED;
    if (proj->x > SCREEN_WIDTH) proj->active = 0;
}

void Projectile_Draw(Projectile_t* proj) {
    if (!proj->active) return;
    LCD_Draw_Sprite_Scaled(proj->x, proj->y,
        PEA_ROWS, PEA_COLS,
        (const uint8_t*)SPRITE_PEA, PEA_SCALE);
}

AABB Projectile_GetAABB(Projectile_t* proj) {
    AABB box;
    box.x      = proj->x;
    box.y      = proj->y;
    box.width  = PEA_COLS * PEA_SCALE;
    box.height = PEA_ROWS * PEA_SCALE;
    return box;
}
