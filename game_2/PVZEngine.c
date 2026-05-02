/**
 * @file PVZEngine.c
 * @brief Plants vs Zombies game engine
 */

#include "PVZEngine.h"
#include "InputHandler.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Plant.h"
#include "Sprites.h"
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  240
#define HUD_HEIGHT      24

#define LED_PORT      GPIOA
#define LED_PIN       GPIO_PIN_5
#define LED_FLASH_MS  80

#define SFX_PLANT_PLACE   600
#define SFX_ZOMBIE_KILL  1200
#define SFX_PEA_FIRE      800
#define SFX_SUN_COLLECT   900
#define SFX_GAME_OVER     200
#define SFX_WAVE_START    500
#define SFX_MENU_OPEN     350
#define SFX_DURATION_MS    60

extern volatile uint8_t joystick_pressed;
static const uint8_t WAVE_ZOMBIE_COUNT[TOTAL_WAVES] = {3, 4, 5, 6, 8};
extern Buzzer_cfg_t buzzer_cfg;

/* ================================================================
 * Buzzer / LED helpers
 * ================================================================ */
static uint32_t g_buzzer_stop = 0;

static void PVZ_Beep(uint32_t freq) {
    buzzer_tone(&buzzer_cfg, freq, 50);  // fixed: was 40, use 50% volume
    g_buzzer_stop = HAL_GetTick() + SFX_DURATION_MS;
}

static void PVZ_UpdateBuzzer(void) {
    if (g_buzzer_stop && (int32_t)(HAL_GetTick() - g_buzzer_stop) >= 0) {
        buzzer_off(&buzzer_cfg);
        g_buzzer_stop = 0;
    }
}

static void PVZ_LED_Flash(PVZEngine_t* e) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
    e->led_off_tick = HAL_GetTick() + LED_FLASH_MS;
}

static void PVZ_UpdateLED(PVZEngine_t* e) {
    uint8_t eating = 0;
    for (int i = 0; i < MAX_ZOMBIES; i++)
        if (e->zombies[i].active && e->zombies[i].eating) { eating = 1; break; }
    if (eating) {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
        e->led_off_tick = 0;
    } else if (e->led_off_tick) {
        if ((int32_t)(HAL_GetTick() - e->led_off_tick) >= 0) {
            HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
            e->led_off_tick = 0;
        }
    } else {
        HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
    }
}

/* ================================================================
 * Pool helpers
 * ================================================================ */
static int PVZ_GetFreeZombieSlot(PVZEngine_t* e) {
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!e->zombies[i].active) return i;
    }
    return -1;
}

static int PVZ_GetFreeProjectileSlot(PVZEngine_t* e) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!e->projectiles[i].active) return i;
    }
    return -1;
}

static int PVZ_GetFreeSunSlot(PVZEngine_t* e) {
    for (int i = 0; i < MAX_SUNS; i++) {
        if (!e->suns[i].active) return i;
    }
    return -1;
}

static int PVZ_PlantAt(PVZEngine_t* e, int16_t col, int16_t row) {
    for (int i = 0; i < MAX_PLANTS; i++)
        if (e->plants[i].active && e->plants[i].grid_col == col && e->plants[i].grid_row == row) return i;
    return -1;
}

static int PVZ_GetFreePlantSlot(PVZEngine_t* e) {
    for (int i = 0; i < MAX_PLANTS; i++) {
        if (!e->plants[i].active) return i;
    }
    return -1;
}

static int PVZ_AliveZombies(PVZEngine_t* e) {
    int n = 0;
    for (int i = 0; i < MAX_ZOMBIES; i++) if (e->zombies[i].active) n++;
    return n;
}

static void PVZ_SpawnZombie(PVZEngine_t* e) {
    int slot = PVZ_GetFreeZombieSlot(e); if (slot < 0) return;
    int16_t lane = (int16_t)(Random_U16(GRID_ROWS));
    ZombieType type = (e->current_wave >= 1 && Random_U16(10) >= 6) ? ZOMBIE_CONE : ZOMBIE_NORMAL;
    Zombie_Init(&e->zombies[slot], type, lane);
}

static void PVZ_DropSun(PVZEngine_t* e) {
    int slot = PVZ_GetFreeSunSlot(e); if (slot < 0) return;
    SunToken_t* s = &e->suns[slot];
    s->active     = 1;
    s->x          = (int16_t)(GRID_ORIGIN_X + Random_U16(GRID_COLS) * CELL_W + 4);
    s->y          = HUD_HEIGHT + 2;
    s->fall_timer = 4;
    s->life_timer = 250;
}

/* ================================================================
 * Input state
 * ================================================================ */
static Direction s_prev_dir = CENTRE;

#define CURSOR_DELAY   14
#define CURSOR_REPEAT   5
static uint8_t s_cur_frames = 0;

static void PVZ_MoveCursor(PVZEngine_t* e, UserInput cur) {
    if (cur.direction == CENTRE) { s_cur_frames = 0; return; }

    uint8_t is_new = (cur.direction != s_prev_dir);
    if (!is_new) {
        s_cur_frames++;
        if (s_cur_frames < CURSOR_DELAY) return;
        if ((s_cur_frames - CURSOR_DELAY) % CURSOR_REPEAT != 0) return;
    } else {
        s_cur_frames = 0;
    }

    switch (cur.direction) {
        case N: case NW: case NE: if (e->cursor_row > 0) e->cursor_row--; break;
        case S: case SW: case SE: if (e->cursor_row < GRID_ROWS - 1) e->cursor_row++; break;
        case E:                   if (e->cursor_col < GRID_COLS - 1) e->cursor_col++; break;
        case W:                   if (e->cursor_col > 0) e->cursor_col--; break;
        default: break;
    }
}

/* ================================================================
 * Sun collection
 * ================================================================ */
static void PVZ_CheckSunCollection(PVZEngine_t* e) {
    int16_t cx = GRID_ORIGIN_X + e->cursor_col * CELL_W;
    int16_t cy = GRID_ORIGIN_Y + e->cursor_row * CELL_H;
    AABB cursor_box = { cx, cy, CELL_W, CELL_H };
    for (int i = 0; i < MAX_SUNS; i++) {
        if (!e->suns[i].active) continue;
        AABB sun_box = { e->suns[i].x, e->suns[i].y, SUN_COLS * 2, SUN_ROWS * 2 };
        if (AABB_Collides(&cursor_box, &sun_box)) {
            e->suns[i].active = 0;
            e->sun += SUN_FALL_VALUE;
            PVZ_Beep(SFX_SUN_COLLECT);
        }
    }
}

/* ================================================================
 * Collision detection
 * ================================================================ */
static void PVZ_CheckProjectileZombieCollisions(PVZEngine_t* e) {
    for (int pi = 0; pi < MAX_PROJECTILES; pi++) {
        if (!e->projectiles[pi].active) continue;
        AABB pa = Projectile_GetAABB(&e->projectiles[pi]);
        for (int zi = 0; zi < MAX_ZOMBIES; zi++) {
            if (!e->zombies[zi].active) continue;
            if (e->zombies[zi].lane != e->projectiles[pi].lane) continue;
            AABB za = Zombie_GetAABB(&e->zombies[zi]);
            if (AABB_Collides(&pa, &za)) {
                Zombie_TakeDamage(&e->zombies[zi], PEA_DAMAGE);
                e->projectiles[pi].active = 0;
                if (!e->zombies[zi].active) {
                    e->score += Zombie_GetScore(&e->zombies[zi]);
                    PVZ_Beep(SFX_ZOMBIE_KILL);
                    PVZ_LED_Flash(e);
                }
                break;
            }
        }
    }
}

static void PVZ_CheckZombiePlantCollisions(PVZEngine_t* e) {
    for (int zi = 0; zi < MAX_ZOMBIES; zi++) {
        if (!e->zombies[zi].active) continue;
        e->zombies[zi].eating = 0;  // reset each frame
        AABB za = Zombie_GetAABB(&e->zombies[zi]);
        for (int pi = 0; pi < MAX_PLANTS; pi++) {
            if (!e->plants[pi].active) continue;
            if (e->plants[pi].grid_row != e->zombies[zi].lane) continue;
            AABB pa = Plant_GetAABB(&e->plants[pi]);
            if (AABB_Collides(&za, &pa)) {
                e->zombies[zi].eating = 1;
                Plant_TakeDamage(&e->plants[pi], ZOMBIE_BITE_DMG);
                break;
            }
        }
    }
}

static void PVZ_CheckZombieReachedHome(PVZEngine_t* e) {
    for (int i = 0; i < MAX_ZOMBIES; i++) {
        if (!e->zombies[i].active) continue;
        if (e->zombies[i].x < 0) {
            e->zombies[i].active = 0;
            if (e->lives > 0) e->lives--;
            if (e->lives == 0) { e->state = STATE_GAME_OVER; PVZ_Beep(SFX_GAME_OVER); }
        }
    }
}

/* ================================================================
 * Wave / spawn
 * ================================================================ */
static void PVZ_TickWave(PVZEngine_t* e) {
    if (e->state != STATE_PLAYING) return;
    if (!e->wave_active) {
        if (e->wave_timer > 0) { e->wave_timer--; return; }
        if (e->current_wave >= TOTAL_WAVES) { e->state = STATE_WIN; return; }
        e->zombies_this_wave = WAVE_ZOMBIE_COUNT[e->current_wave];
        e->spawn_timer       = SPAWN_INTERVAL;
        e->wave_active       = 1;
        e->current_wave++;
        PVZ_Beep(SFX_WAVE_START);
        return;
    }
    if (e->zombies_this_wave > 0) {
        if (e->spawn_timer > 0) e->spawn_timer--;
        else { PVZ_SpawnZombie(e); e->zombies_this_wave--; e->spawn_timer = SPAWN_INTERVAL; }
    }
    if (e->zombies_this_wave == 0 && PVZ_AliveZombies(e) == 0) {
        e->wave_active = 0; e->wave_timer = WAVE_INTERVAL;
    }
}

static uint16_t s_sun_fall_timer = SUN_FALL_INTERVAL;
static void PVZ_TickAutoSun(PVZEngine_t* e) {
    if (e->state != STATE_PLAYING) return;
    if (s_sun_fall_timer > 0) s_sun_fall_timer--;
    else { PVZ_DropSun(e); s_sun_fall_timer = SUN_FALL_INTERVAL; }
}

/* ================================================================
 * Place armed plant at cursor
 * ================================================================ */
static uint8_t PVZ_TryPlace(PVZEngine_t* e) {
    if (!e->plant_armed || e->selected_type == PLANT_NONE) return 0;
    int16_t cost = 0;
    switch (e->selected_type) {
        case PLANT_PEASHOOTER: cost = COST_PEASHOOTER; break;
        case PLANT_SUNFLOWER:  cost = COST_SUNFLOWER;  break;
        case PLANT_WALLNUT:    cost = COST_WALLNUT;    break;
        default: return 0;
    }
    if (e->sun < (uint16_t)cost) return 0;
    if (PVZ_PlantAt(e, e->cursor_col, e->cursor_row) >= 0) return 0;
    int slot = PVZ_GetFreePlantSlot(e); if (slot < 0) return 0;
    Plant_Init(&e->plants[slot], e->selected_type, e->cursor_col, e->cursor_row);
    e->sun       -= (uint16_t)cost;
    e->plant_armed = 0;
    PVZ_Beep(SFX_PLANT_PLACE);
    return 1;
}

/* ================================================================
 * Public API
 * ================================================================ */
void PVZEngine_Init(PVZEngine_t* engine) {
    memset(engine, 0, sizeof(PVZEngine_t));
    engine->state         = STATE_PLAYING;
    engine->sun           = SUN_START;
    engine->lives         = 5;
    engine->wave_timer    = 150;
    engine->spawn_timer   = SPAWN_INTERVAL;
    engine->selected_type = PLANT_PEASHOOTER;
    engine->plant_armed   = 0;

    s_sun_fall_timer = SUN_FALL_INTERVAL;
    s_prev_dir       = CENTRE;
    s_cur_frames     = 0;
    g_buzzer_stop    = 0;
}

void PVZEngine_Update(PVZEngine_t* engine, UserInput input) {

    if (engine->state == STATE_GAME_OVER || engine->state == STATE_WIN) {
        PVZ_UpdateBuzzer();
        PVZ_UpdateLED(engine);
        s_prev_dir = input.direction;
        return;
    }
    if(current_input.btn3_pressed){
        joystick_pressed = 1;
    }
    uint8_t click_event = joystick_pressed;
    joystick_pressed = 0;

    /* STATE_MENU */
    if (engine->state == STATE_MENU) {
        if (input.direction == N && s_prev_dir != N) {
            if (engine->selected_type > PLANT_PEASHOOTER) engine->selected_type--;
        }
        if (input.direction == S && s_prev_dir != S) {
            if (engine->selected_type < PLANT_CHERRY_BOMB) engine->selected_type++;
        }
        if (click_event) {
            engine->plant_armed = 1;
            engine->state       = STATE_PLAYING;
            s_cur_frames        = 0;
        }
        s_prev_dir = input.direction;
        PVZ_UpdateBuzzer();
        PVZ_UpdateLED(engine);
        return;
    }

    /* STATE_PLAYING */
    PVZ_MoveCursor(engine, input);

    if (click_event) {
        if (engine->plant_armed) {
            PVZ_TryPlace(engine);
        } else {
            engine->state = STATE_MENU;
            PVZ_Beep(SFX_MENU_OPEN);
        }
    }

    s_prev_dir = input.direction;

    PVZ_CheckSunCollection(engine);

    for (int i = 0; i < MAX_PLANTS; i++) {
        if (!engine->plants[i].active) continue;
        Plant_Update(&engine->plants[i]);

        if (engine->plants[i].shoot_ready) {
            int slot = PVZ_GetFreeProjectileSlot(engine);
            if (slot >= 0) {
                    if (engine->plants[i].shoot_ready) {
                    /* check if any active zombie is in the same lane */
                    uint8_t zombie_in_lane = 0;
                    for (int zi = 0; zi < MAX_ZOMBIES; zi++) {
                        if (engine->zombies[zi].active &&
                            engine->zombies[zi].lane == engine->plants[i].grid_row) {
                            zombie_in_lane = 1;
                            break;
                        }
                    }

                    if (zombie_in_lane) {
                        int slot = PVZ_GetFreeProjectileSlot(engine);
                        if (slot >= 0) {
                            int16_t px = engine->plants[i].x + (PEASHOOTER_COLS * PLANT_SCALE);
                            int16_t py = engine->plants[i].y + (PEASHOOTER_ROWS * PLANT_SCALE) / 2 - (PEA_ROWS * 2) / 2;
                            Projectile_Fire(&engine->projectiles[slot], px, py, engine->plants[i].grid_row);
                            PVZ_Beep(SFX_PEA_FIRE);
                        }
                    }
                }

                int16_t px = engine->plants[i].x + (PEASHOOTER_COLS * PLANT_SCALE);
                int16_t py = engine->plants[i].y + (PEASHOOTER_ROWS * PLANT_SCALE) / 2 - (PEA_ROWS * 2) / 2;
                Projectile_Fire(&engine->projectiles[slot], px, py, engine->plants[i].grid_row);
                PVZ_Beep(SFX_PEA_FIRE);
            }
        }
        if (engine->plants[i].sun_ready) {
            int slot = PVZ_GetFreeSunSlot(engine);
            if (slot >= 0) {
                SunToken_t* s = &engine->suns[slot];
                s->active = 1;
                s->x = engine->plants[i].x + 4;
                s->y = engine->plants[i].y + (PEASHOOTER_ROWS * PLANT_SCALE) + 2;
                s->fall_timer = 5;
                s->life_timer = 250;
            }
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) Projectile_Update(&engine->projectiles[i]);
    for (int i = 0; i < MAX_ZOMBIES;     i++) Zombie_Update(&engine->zombies[i]);

    for (int i = 0; i < MAX_SUNS; i++) {
        if (!engine->suns[i].active) continue;
        SunToken_t* s = &engine->suns[i];
        if (s->fall_timer > 0) s->fall_timer--; else { s->y++; s->fall_timer = 4; }
        if (s->y > SCREEN_HEIGHT - SUN_ROWS * 2 - 2) s->y = SCREEN_HEIGHT - SUN_ROWS * 2 - 2;
        if (s->life_timer > 0) s->life_timer--; else s->active = 0;
    }

    PVZ_CheckProjectileZombieCollisions(engine);
    PVZ_CheckZombiePlantCollisions(engine);
    PVZ_CheckZombieReachedHome(engine);
    PVZ_TickWave(engine);
    PVZ_TickAutoSun(engine);
    PVZ_UpdateBuzzer();
    PVZ_UpdateLED(engine);
}

/* ================================================================
 * Draw helpers
 * ================================================================ */
static void PVZ_DrawHUD(PVZEngine_t* e) {
    char buf[24];
    LCD_printString("SUN:", 0, 2, 6, 2);
    sprintf(buf, "%d", e->sun);
    LCD_printString(buf, 42, 2, 6, 2);
    LCD_printString("SC:", 85, 2, 1, 2);
    sprintf(buf, "%lu", e->score);
    LCD_printString(buf, 115, 2, 1, 2);
    LCD_printString("W:", 158, 2, 5, 2);
    sprintf(buf, "%d/%d", e->current_wave, TOTAL_WAVES);
    LCD_printString(buf, 175, 2, 5, 2);
    LCD_printString("LV:", 205, 2, 2, 2);
    sprintf(buf, "%d", e->lives);
    LCD_printString(buf, 228, 2, 2, 2);
    LCD_Draw_Line(0, HUD_HEIGHT, SCREEN_WIDTH, HUD_HEIGHT, 13);
}

static void PVZ_DrawMenu(PVZEngine_t* e) {
    /* solid backdrop — drawn over everything including lawn */
    LCD_Draw_Rect(10, 40, 225, 195, 0, 1);   // black fill

    LCD_printString("PICK PLANT", 50, 58, 2, 2);
    LCD_printString("N/S:scroll btn:ok", 25, 78, 3, 2);

    struct { PlantType t; const char* n; uint16_t c; uint8_t col; } options[] = {
        { PLANT_PEASHOOTER, "Peashooter 100", COST_PEASHOOTER, 12  },
        { PLANT_SUNFLOWER,  "Sunflower   50", COST_SUNFLOWER,  12 },
        { PLANT_WALLNUT,    "Wallnut     50", COST_WALLNUT,    12 },
        {PLANT_A_PEASHOOTER, "A.Peashooter 150", COST_A_PEASHOOTER, 12},
        {PLANT_CHERRY_BOMB,  "Cherry Bomb 150", COST_CHERRY_BOMB, 12}
    };
    for (int i = 0; i < 5; i++) {
        int16_t y = 100 + i * 24;  
        uint8_t sel = (e->selected_type == options[i].t);
        if (sel) LCD_Draw_Rect(25, y - 2, 195, 24, options[i].col, 0);
        LCD_printString(options[i].n, 30, y, sel ? options[i].col : 13, 2);
        if (e->sun < options[i].c) LCD_printString("$$", 185, y, 8, 2);
    }
}

static void PVZ_DrawCursor(PVZEngine_t* e) {
    int16_t cx = GRID_ORIGIN_X + e->cursor_col * CELL_W;
    int16_t cy = GRID_ORIGIN_Y + e->cursor_row * CELL_H;
    uint8_t col = e->plant_armed ? 3 : 2;
    LCD_Draw_Rect(cx, cy, CELL_W, CELL_H, col, 0);
}

/* ================================================================
 * PVZEngine_Draw
 * ================================================================ */
void PVZEngine_Draw(PVZEngine_t* engine) {

    /* HUD bar */
    LCD_Draw_Rect(0, 0, SCREEN_WIDTH, HUD_HEIGHT, 0, 1);
    PVZ_DrawHUD(engine);

    /* Game objects */
    for (int i = 0; i < MAX_PLANTS;      i++) Plant_Draw(&engine->plants[i]);
    for (int i = 0; i < MAX_ZOMBIES;     i++) Zombie_Draw(&engine->zombies[i]);
    for (int i = 0; i < MAX_PROJECTILES; i++) Projectile_Draw(&engine->projectiles[i]);

    /* Sun tokens */
    for (int i = 0; i < MAX_SUNS; i++) {
        if (!engine->suns[i].active) continue;
        LCD_Draw_Sprite_Scaled(engine->suns[i].x, engine->suns[i].y,
            SUN_ROWS, SUN_COLS, (const uint8_t*)SPRITE_SUN, 2);
    }

    PVZ_DrawCursor(engine);

    

    /* Overlays — drawn last so they appear on top of everything */
    switch (engine->state) {
        case STATE_MENU:
            PVZ_DrawMenu(engine);
            break;
        case STATE_GAME_OVER:
            LCD_Draw_Rect(20, 80, 200, 85, 0, 1);
            LCD_Draw_Rect(20, 80, 200, 85, 2, 0);
            LCD_printString("GAME OVER", 38, 90, 2, 3);
            { char buf[24]; sprintf(buf, "Score: %lu", engine->score);
              LCD_printString(buf, 45, 140, 1, 2); }
            break;
        case STATE_WIN:
            LCD_Draw_Rect(20, 80, 200, 85, 0, 1);
            LCD_Draw_Rect(20, 80, 200, 85, 3, 0);
            LCD_printString("YOU WIN!", 48, 90, 3, 3);
            { char buf[24]; sprintf(buf, "Score: %lu", engine->score);
              LCD_printString(buf, 45, 140, 6, 2); }
            break;
        default: break;
    }
}

uint16_t    PVZEngine_GetSun(PVZEngine_t* e)   { return e->sun;   }
uint32_t    PVZEngine_GetScore(PVZEngine_t* e) { return e->score; }
GameState_t PVZEngine_GetState(PVZEngine_t* e) { return e->state; }