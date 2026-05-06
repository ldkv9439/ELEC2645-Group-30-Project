/**
 * @file PVZEngine.h
 * @brief Main game engine for Plants vs Zombies
 *
 * FLOW:
 *   STATE_PLAYING    – game runs; cursor moves with joystick
 *                      • push joystick (any dir from centre) → STATE_MENU
 *                      • if plant armed: push on empty cell = place it
 *   STATE_MENU       – overlay showing plant list; game PAUSED
 *                      • N/S scroll selection
 *                      • push joystick again → close menu, arm selected plant, back to PLAYING
 *   STATE_GAME_OVER  – zombie reached home
 *   STATE_WIN        – all waves cleared
 *
 * JOYSTICK MAPPING:
 *   Move             – move cursor (in PLAYING) / scroll list (in MENU)
 *   Centre push      – open menu (PLAYING, no plant armed)
 *                    – confirm/close menu (MENU)
 *                    – place plant (PLAYING, plant armed, valid cell)
 *   Any push         – treated as "button press" when released back to CENTRE
 */

#ifndef PVZENGINE_H
#define PVZENGINE_H

#include <stdint.h>
#include "Plant.h"
#include "Zombie.h"
#include "Projectile.h"
#include "Utils.h"
#include "Joystick.h"

/* ---- Pool sizes ---- */
#define MAX_PLANTS       36
#define MAX_ZOMBIES      12
#define MAX_PROJECTILES  20
#define MAX_SUNS         10

/* ---- Sun economy ---- */
#define SUN_START         150
#define SUN_FALL_INTERVAL 100
#define SUN_FALL_VALUE     25

/* ---- Wave system ---- */
#define TOTAL_WAVES        5
#define WAVE_INTERVAL    100
#define SPAWN_INTERVAL   60

/* ---- Sun collectible ---- */
typedef struct {
    uint8_t  active;
    int16_t  x;
    int16_t  y;
    uint8_t  fall_timer;
    uint8_t  life_timer;
} SunToken_t;

/* ---- Game states ---- */
typedef enum {
    STATE_PLAYING   = 0,   /* normal gameplay                  */
    STATE_MENU      = 1,   /* plant-select overlay (paused)    */
    STATE_GAME_OVER = 2,
    STATE_WIN       = 3,
    STATE_WAVE_ANNOUNCE = 4,
} GameState_t;

/**
 * @struct PVZEngine_t
 */
typedef struct {
    Plant       plants[MAX_PLANTS];
    Zombie      zombies[MAX_ZOMBIES];
    Projectile_t  projectiles[MAX_PROJECTILES];
    SunToken_t    suns[MAX_SUNS];

    GameState_t   state;
    uint16_t      sun;
    uint32_t      score;
    uint8_t       lives;
    uint8_t       current_wave;
    uint16_t      wave_timer;
    uint16_t      spawn_timer;
    uint8_t       zombies_this_wave;
    uint8_t       wave_active;
    uint8_t       wave_announce;

    /* Cursor */
    int16_t       cursor_col;
    int16_t       cursor_row;

    /* Plant selection */
    PlantType   selected_type;  /* PLANT_NONE = nothing armed  */
    uint8_t       plant_armed;    /* 1 = player has chosen a plant to place */

    /* LED */
    uint32_t      led_off_tick;
} PVZEngine_t;

void        PVZEngine_Init(PVZEngine_t* engine);
void        PVZEngine_Update(PVZEngine_t* engine, UserInput input);
void        PVZEngine_Draw(PVZEngine_t* engine);
uint16_t    PVZEngine_GetSun(PVZEngine_t* engine);
uint32_t    PVZEngine_GetScore(PVZEngine_t* engine);
GameState_t PVZEngine_GetState(PVZEngine_t* engine);

#endif /* PVZENGINE_H */