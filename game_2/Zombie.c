/**
 * @file Zombie.c
 * @brief moves left each frame unless eating a plant,
 *        draws normal or cone zombie sprite at 2x scale with HP bar,
 *        and provides damage, AABB, and score functions.
 */
#include "Zombie.h"
#include "LCD.h"
#include "Sprites.h"
#include "Plant.h"   /* GRID_ORIGIN_Y, CELL_H */

#define SCREEN_WIDTH  240

/* Scaled dimensions */
#define ZW  (ZOMBIE_COLS * ZOMBIE_SCALE)
#define ZH  (ZOMBIE_ROWS * ZOMBIE_SCALE)
#define CZH (CONE_ZOMBIE_ROWS * ZOMBIE_SCALE)

void Zombie_Init(Zombie* zombie, ZombieType type, int16_t lane) {
    zombie->type       = type;
    zombie->active     = 1;
    zombie->lane       = lane;
    zombie->eating     = 0;
    zombie->move_timer = 0;

    zombie->x = SCREEN_WIDTH;
    /* Centre zombie vertically in grid row */
    zombie->y = GRID_ORIGIN_Y + lane * CELL_H + (CELL_H - ZH) / 2;

    zombie->hp_max = (type == ZOMBIE_CONE) ? HP_ZOMBIE_CONE : HP_ZOMBIE_NORMAL;
    zombie->hp     = zombie->hp_max;
}

void Zombie_Update(Zombie* zombie) {
    if (!zombie->active) return;

    /* If eating, stay still — PVZEngine sets eating=1 each frame a plant
       is in contact. The zombie only moves when eating == 0.             */
    if (zombie->eating) return;

    uint8_t speed_div = (zombie->type == ZOMBIE_CONE)
                        ? ZOMBIE_SPEED_DIV_CONE
                        : ZOMBIE_SPEED_DIV_NORMAL;
    zombie->move_timer++;
    if (zombie->move_timer >= speed_div) {
        zombie->move_timer = 0;
        zombie->x--;
    }
}

void Zombie_Draw(Zombie* zombie) {
    if (!zombie->active) return;

    if (zombie->type == ZOMBIE_CONE) {
        LCD_Draw_Sprite_Scaled(zombie->x, zombie->y, ZOMBIE_ROWS,
            ZOMBIE_COLS, (const uint8_t*)SPRITE_ZOMBIE, ZOMBIE_SCALE);
    } else {
        LCD_Draw_Sprite_Scaled(zombie->x, zombie->y, ZOMBIE_ROWS,
            ZOMBIE_COLS, (const uint8_t*)SPRITE_CONE_ZOMBIE, ZOMBIE_SCALE);
    }

    /* HP bar */
    if (zombie->hp < zombie->hp_max) {
        int16_t bar_w  = ZW;
        int16_t filled = (int16_t)((int32_t)bar_w * zombie->hp / zombie->hp_max);
        int16_t bar_y  = zombie->y + ZH + 1;
        LCD_Draw_Rect(zombie->x, bar_y, bar_w, 3, 2, 1);
        if (filled > 0)
            LCD_Draw_Rect(zombie->x, bar_y, filled, 3, 3, 1);
    }
}

void Zombie_TakeDamage(Zombie* zombie, int16_t dmg) {
    if (!zombie->active) return;
    zombie->hp -= dmg;
    if (zombie->hp <= 0) { zombie->hp = 0; zombie->active = 0; }
}

AABB Zombie_GetAABB(Zombie* zombie) {
    AABB box;
    box.x      = zombie->x;
    box.y      = zombie->y;
    box.width  = ZW;
    box.height = ZH;
    return box;
}

uint16_t Zombie_GetScore(Zombie* zombie) {
    return (zombie->type == ZOMBIE_CONE) ? SCORE_ZOMBIE_CONE : SCORE_ZOMBIE_NORMAL;
}