/**
 * @file Plant.h
 * @brief Plant objects for Plants vs Zombies
 *
 * Grid: 6 rows x 6 cols, each cell 25x30 px.
 * Sprites drawn at 2x scale (16px -> 32px) centred in each cell.
 */
#ifndef PLANT_H
#define PLANT_H

#include <stdint.h>
#include "Utils.h"

/* ---- Grid layout ---- */
#define GRID_COLS        6
#define GRID_ROWS        6
#define CELL_W          25
#define CELL_H          30
#define GRID_ORIGIN_X   37
#define GRID_ORIGIN_Y   51

/* ---- Sprite scale ---- */
#define PLANT_SCALE      2   /* 16px drawn as 32px on screen */

/* ---- Plant types ---- */
typedef enum {
    PLANT_NONE       = 0,
    PLANT_PEASHOOTER = 1,
    PLANT_SUNFLOWER  = 2,
    PLANT_WALLNUT    = 3,
    PLANT_A_PEASHOOTER = 4,
    PLANT_CHERRY_BOMB  = 5
} PlantType;

#define COST_PEASHOOTER 75
#define COST_SUNFLOWER   50
#define COST_WALLNUT     50
#define COST_A_PEASHOOTER 150
#define COST_CHERRY_BOMB  125

#define HP_PEASHOOTER  300
#define HP_SUNFLOWER   200
#define HP_WALLNUT     900
#define HP_A_PEASHOOTER 500
#define HP_CHERRY_BOMB  200

#define SHOOT_INTERVAL_PEASHOOTER  40
#define SHOOT_INTERVAL_A_PEASHOOTER  20
#define SUN_INTERVAL_SUNFLOWER    160
#define EXPLODE_DURATION_CHERRY_BOMB 40

typedef struct {
    PlantType type;
    uint8_t     active;
    int16_t     grid_col;
    int16_t     grid_row;
    int16_t     x;
    int16_t     y;
    int16_t     hp;
    int16_t     hp_max;
    uint16_t    timer;
    uint8_t     shoot_ready;
    uint8_t     sun_ready;
    uint8_t     explode_ready;
    uint8_t     explode_timer;
} Plant;

void Plant_Init(Plant* plant, PlantType type, int16_t col, int16_t row);
void Plant_Update(Plant* plant);
void Plant_Draw(Plant* plant);
void Plant_TakeDamage(Plant* plant, int16_t dmg);
AABB Plant_GetAABB(Plant* plant);

#endif /* PLANT_H */
