/**
 * @file Projectile.c
 * @brief Pea projectile - fires, moves right across the screen, draws normal
 *        or blue pea sprite based on type, and provides AABB for collision detection.
 */
#include "Projectile.h"
#include "LCD.h"
#include "Sprites.h"

#define SCREEN_WIDTH 240
#define PEA_SCALE    2

void Projectile_Fire(Projectile_t* proj, int16_t x, int16_t y, int16_t lane, PeaType pea_type) {
    proj->active = 1;
    proj->x      = x;
    proj->y      = y;
    proj->lane   = lane;
    proj->pea_type   = pea_type;
}

void Projectile_Update(Projectile_t* proj) {
    if (!proj->active) return;
    proj->x += PEA_SPEED;
    if (proj->x > SCREEN_WIDTH) proj->active = 0;
}

void Projectile_Draw(Projectile_t* proj) {
    if (!proj->active) return;
    const uint8_t* spr = (proj->pea_type == PEA_BLUE) ? (const uint8_t*)A_SPRITE_PEA : (const uint8_t*)SPRITE_PEA;
    LCD_Draw_Sprite_Scaled(proj->x, proj->y, PEA_ROWS, PEA_COLS, spr, PEA_SCALE);
}

AABB Projectile_GetAABB(Projectile_t* proj) {
    AABB box;
    box.x      = proj->x;
    box.y      = proj->y;
    box.width  = PEA_COLS * PEA_SCALE;
    box.height = PEA_ROWS * PEA_SCALE;
    return box;
}
