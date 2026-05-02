#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "PWM.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
extern PWM_cfg_t pwm_cfg;

/*
 * 5-LEVEL MAZE GAME
 * ------------------
 * Joystick: move player 
 * BT3: return to menu
 * 13x13 mazes
 * Timer gets shorter on harder levels
 * If time runs out -> loud fail sound and restart from level 1
 *
 * LED behaviour:
 * - External PWM LED only
 * - Brighter each level
 * - Blinks when 15 seconds or less remain
 *
 * High score:
 * - Lowest total moves wins
 */

#define GAME1_FRAME_TIME_MS 30
#define MOVE_DELAY_MS 140

#define LEVEL_COUNT 5

#define MAZE_ROWS 13
#define MAZE_COLS 13
#define TILE_SIZE 14

#define MAZE_X0 29
#define MAZE_Y0 18

#define CELL_EMPTY 0
#define CELL_WALL  1
#define CELL_EXIT  2

#define LOW_TIME_WARNING 15

static const uint8_t level_time_limits[LEVEL_COUNT] = {60, 35, 28, 22, 18};
static const uint8_t level_led_brightness[LEVEL_COUNT] = {20, 35, 50, 70, 90};

// 5 fixed levels (13x13) - SOLVABLE LEVELS
static const uint8_t maze_levels[LEVEL_COUNT][MAZE_ROWS][MAZE_COLS] = {
    // ---------- Level 1 ----------
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // ---------- Level 2 ----------
{
    {1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,0,1,0,1,0,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,1,0,1},
    {1,1,1,1,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1}
},

    // ---------- Level 3 ----------
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,1,0,1,0,1,0,1},
        {1,0,1,1,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,1},
        {1,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,2,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // ---------- Level 4 ----------
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,1,0,1,0,0,0,0,0,1,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1},
        {1,0,1,0,1,0,1,0,0,0,1,0,1},
        {1,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,1,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // ---------- Level 5 ----------
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,0,1,1,1,1,1,0,1,1,1},
        {1,0,0,0,1,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,0,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

// Game state
static uint8_t current_level = 0;
static uint8_t player_row = 1;
static uint8_t player_col = 1;
static uint8_t all_levels_cleared = 0;

static uint32_t level_moves = 0;
static uint32_t total_moves = 0;
static uint32_t last_move_tick = 0;

static uint32_t level_start_tick = 0;
static uint8_t time_left_seconds = 60;

// High score state
static uint32_t best_total_moves = 0;
static uint8_t best_score_set = 0;

// ---------- Helper functions ----------

static void Update_Level_LED(void)
{
    if (all_levels_cleared) {
        PWM_SetDuty(&pwm_cfg, 100);
        return;
    }

    uint8_t base_brightness = level_led_brightness[current_level];

    if (time_left_seconds <= LOW_TIME_WARNING) {
        if (((HAL_GetTick() / 200) % 2) == 0) {
            PWM_SetDuty(&pwm_cfg, base_brightness);
        } else {
            PWM_SetDuty(&pwm_cfg, 0);
        }
    } else {
        PWM_SetDuty(&pwm_cfg, base_brightness);
    }
}

static void Maze_ResetPlayer(void)
{
    player_row = 1;
    player_col = 1;
    level_moves = 0;
    last_move_tick = 0;
    level_start_tick = HAL_GetTick();
    time_left_seconds = level_time_limits[current_level];
    Update_Level_LED();
}

static void Reset_To_Level_1(void)
{
    current_level = 0;
    total_moves = 0;
    all_levels_cleared = 0;
    Maze_ResetPlayer();
}

static uint8_t Maze_Cell(int16_t row, int16_t col)
{
    return maze_levels[current_level][row][col];
}

static uint8_t Maze_IsWall(int16_t row, int16_t col)
{
    if (row < 0 || row >= MAZE_ROWS || col < 0 || col >= MAZE_COLS) {
        return 1;
    }

    return (Maze_Cell(row, col) == CELL_WALL);
}

static void Play_Wall_Oops(void)
{
    buzzer_tone(&buzzer_cfg, 400, 50);
    HAL_Delay(35);
    buzzer_tone(&buzzer_cfg, 550, 50);
    HAL_Delay(35);
    buzzer_tone(&buzzer_cfg, 700, 45);
    HAL_Delay(45);
    buzzer_off(&buzzer_cfg);
}

static void Play_Level_Clear_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 900, 30);
    HAL_Delay(60);
    buzzer_tone(&buzzer_cfg, 1200, 30);
    HAL_Delay(70);
    buzzer_off(&buzzer_cfg);
}

static void Play_Game_Clear_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 900, 35);
    HAL_Delay(80);
    buzzer_tone(&buzzer_cfg, 1200, 35);
    HAL_Delay(80);
    buzzer_tone(&buzzer_cfg, 1600, 35);
    HAL_Delay(120);
    buzzer_off(&buzzer_cfg);
}

static void Play_Fail_Bleep(void)
{
    buzzer_tone(&buzzer_cfg, 950, 90);
    HAL_Delay(500);
    buzzer_off(&buzzer_cfg);
}

static void Show_Level_Clear_Screen(void)
{
    char msg[32];

    LCD_Fill_Buffer(0);
    LCD_printString("LEVEL CLEAR!", 45, 80, 3, 2);

    sprintf(msg, "Next Level: %d", current_level + 2);
    LCD_printString(msg, 55, 120, 1, 1);

    LCD_Refresh(&cfg0);
    HAL_Delay(700);
}

static void Show_Fail_Screen(void)
{
    LCD_Fill_Buffer(0);
    LCD_printString("TIME UP!", 70, 80, 2, 2);
    LCD_printString("Back to Level 1", 45, 120, 1, 1);
    LCD_Refresh(&cfg0);
    HAL_Delay(900);
}

static void Maze_TryMove(int16_t d_row, int16_t d_col)
{
    int16_t new_row = (int16_t)player_row + d_row;
    int16_t new_col = (int16_t)player_col + d_col;

    if (!Maze_IsWall(new_row, new_col)) {
        player_row = (uint8_t)new_row;
        player_col = (uint8_t)new_col;
        level_moves++;
        total_moves++;

        if (Maze_Cell(player_row, player_col) == CELL_EXIT) {
            if (current_level < (LEVEL_COUNT - 1)) {
                Play_Level_Clear_Sound();
                Show_Level_Clear_Screen();
                current_level++;
                Maze_ResetPlayer();
            } else {
                all_levels_cleared = 1;

                if (!best_score_set || total_moves < best_total_moves) {
                    best_total_moves = total_moves;
                    best_score_set = 1;
                }

                Play_Game_Clear_Sound();
                Update_Level_LED();
            }
        }
    } else {
        Play_Wall_Oops();
    }
}

static void Maze_UpdateTimer(void)
{
    if (all_levels_cleared) {
        return;
    }

    uint32_t elapsed_ms = HAL_GetTick() - level_start_tick;
    uint32_t elapsed_seconds = elapsed_ms / 1000;
    uint8_t current_limit = level_time_limits[current_level];

    if (elapsed_seconds >= current_limit) {
        time_left_seconds = 0;
        Update_Level_LED();
        Play_Fail_Bleep();
        Show_Fail_Screen();
        Reset_To_Level_1();
        return;
    }

    time_left_seconds = (uint8_t)(current_limit - elapsed_seconds);
}

static void Maze_Update(void)
{
    if (all_levels_cleared) {
        Update_Level_LED();
        return;
    }

    Maze_UpdateTimer();
    Update_Level_LED();

    uint32_t now = HAL_GetTick();

    if ((now - last_move_tick) < MOVE_DELAY_MS) {
        return;
    }

    Joystick_Read(&joystick_cfg, &joystick_data);
    UserInput joy = Joystick_GetInput(&joystick_data);

    switch (joy.direction) {
        case N:
            Maze_TryMove(-1, 0);
            last_move_tick = now;
            break;
        case S:
            Maze_TryMove(1, 0);
            last_move_tick = now;
            break;
        case E:
            Maze_TryMove(0, 1);
            last_move_tick = now;
            break;
        case W:
            Maze_TryMove(0, -1);
            last_move_tick = now;
            break;
        default:
            break;
    }
}

static void Maze_Draw(void)
{
    LCD_Fill_Buffer(0);

    if (all_levels_cleared) {
        char total_str[32];
        char best_str[32];

        LCD_printString("ALL 5 LEVELS", 52, 55, 3, 2);
        LCD_printString("CLEARED!", 72, 85, 3, 2);

        sprintf(total_str, "Your Moves: %lu", (unsigned long)total_moves);
        LCD_printString(total_str, 40, 125, 1, 1);

        if (best_score_set) {
            sprintf(best_str, "High Score: %lu", (unsigned long)best_total_moves);
            LCD_printString(best_str, 40, 145, 1, 1);
        }

        LCD_printString("Lower moves = better", 35, 170, 1, 1);
        LCD_printString("Press BT3 for Menu", 42, 195, 1, 1);
        LCD_Refresh(&cfg0);
        return;
    }

    char title[32];
    sprintf(title, "MAZE - LEVEL %d", current_level + 1);
    LCD_printString(title, 40, 4, 1, 2);

    for (uint8_t row = 0; row < MAZE_ROWS; row++) {
        for (uint8_t col = 0; col < MAZE_COLS; col++) {
            uint16_t x = MAZE_X0 + col * TILE_SIZE;
            uint16_t y = MAZE_Y0 + row * TILE_SIZE;

            if (Maze_Cell(row, col) == CELL_WALL) {
                LCD_Draw_Rect(x, y, TILE_SIZE, TILE_SIZE, 4, 1);
            } else if (Maze_Cell(row, col) == CELL_EXIT) {
                LCD_Draw_Rect(x, y, TILE_SIZE, TILE_SIZE, 3, 1);
                LCD_Draw_Rect(x + 3, y + 3, TILE_SIZE - 6, TILE_SIZE - 6, 1, 0);
            } else {
                LCD_Draw_Rect(x, y, TILE_SIZE, TILE_SIZE, 13, 0);
            }
        }
    }

    {
        uint16_t px = MAZE_X0 + player_col * TILE_SIZE + 3;
        uint16_t py = MAZE_Y0 + player_row * TILE_SIZE + 3;
        LCD_Draw_Rect(px, py, TILE_SIZE - 6, TILE_SIZE - 6, 6, 1);
    }

    char info1[32];
    char info2[32];
    char info3[32];

    sprintf(info1, "L:%lu", (unsigned long)level_moves);
    sprintf(info2, "T:%lu", (unsigned long)total_moves);
    sprintf(info3, "%us", time_left_seconds);

    LCD_printString(info1, 8, 208, 1, 1);
    LCD_printString(info2, 70, 208, 1, 1);
    LCD_printString(info3, 140, 208, 2, 1);
    LCD_printString("BT3", 190, 208, 1, 1);

    LCD_Refresh(&cfg0);
}

// ---------- Main game function ----------

MenuState Game1_Run(void)
{
    Reset_To_Level_1();

    buzzer_tone(&buzzer_cfg, 1000, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);

    MenuState exit_state = MENU_STATE_HOME;

    while (1) {
        uint32_t frame_start = HAL_GetTick();

        Input_Read();

        if (current_input.btn3_pressed) {
            PWM_SetDuty(&pwm_cfg, 0);
            exit_state = MENU_STATE_HOME;
            break;
        }

        Maze_Update();
        Maze_Draw();

        uint32_t frame_time = HAL_GetTick() - frame_start;
        if (frame_time < GAME1_FRAME_TIME_MS) {
            HAL_Delay(GAME1_FRAME_TIME_MS - frame_time);
        }
    }

    return exit_state;
}
