#include "Game_1.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "PWM.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

// stuff from main.c
extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern Joystick_t joystick_data;
extern PWM_cfg_t pwm_cfg;

// game speed settings
#define FRAME_TIME 30
#define MOVE_DELAY 140

// total number of levels
#define LEVELS 5

// maze size
#define ROWS 13
#define COLS 13
#define SIZE 14

// where the maze starts drawing on the LCD
#define START_X 31
#define START_Y 26

// different things each cell can be
#define EMPTY 0
#define WALL  1
#define EXIT  2
#define COIN  3

// when time is low the LED starts blinking
#define LOW_TIME 15

// time allowed for each level
static const uint8_t level_time[LEVELS] = {60, 35, 28, 22, 18};

// LED brightness for each level
static const uint8_t led_level[LEVELS] = {20, 35, 50, 70, 90};

// all maze layouts stored here
static const uint8_t mazes[LEVELS][ROWS][COLS] = {
    // level 1
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,3,0,0,1,0,0,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,3,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,3,0,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // level 2
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,3,0,0,0,1,0,0,0,1},
        {1,1,1,0,1,0,1,0,1,0,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,0,3,1},
        {1,0,1,1,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,1,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,3,1,0,0,0,1,0,1},
        {1,0,1,1,1,0,1,0,1,1,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,2,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // level 3
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,1,0,0,0,3,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,1,0,1,0,1,0,1},
        {1,0,1,1,1,0,1,0,1,0,1,0,1},
        {1,0,0,0,1,0,0,0,0,0,1,3,1},
        {1,1,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,1,0,0,0,0,0,1,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,3,2,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // level 4
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,1,0,0,0,0,0,0,0,3,0,1},
        {1,0,1,0,1,1,1,1,1,1,1,0,1},
        {1,0,1,0,1,0,0,0,0,0,1,0,1},
        {1,0,1,0,1,0,1,1,1,0,1,0,1},
        {1,0,1,0,1,0,1,0,0,0,1,0,1},
        {1,0,1,0,1,0,1,0,1,1,1,0,1},
        {1,0,0,0,1,0,1,0,0,3,0,0,1},
        {1,1,1,1,1,0,1,1,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,3,0,1,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    },

    // level 5
    {
        {1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,0,0,0,1,0,0,3,1},
        {1,0,1,0,1,0,1,0,1,0,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,0,1,1,1,1,1,0,1,1,1},
        {1,0,0,0,1,0,0,0,3,0,1,0,1},
        {1,0,1,1,1,0,1,1,1,1,1,0,1},
        {1,0,1,0,0,0,1,0,0,0,0,0,1},
        {1,0,1,0,1,1,1,0,1,1,1,0,1},
        {1,0,0,3,0,0,0,0,1,0,0,2,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
};

// this is the maze we actually use while playing
// so coins can disappear after collecting them
static uint8_t maze_now[ROWS][COLS];

// game variables
static uint8_t level_num = 0;
static uint8_t player_r = 1;
static uint8_t player_c = 1;
static uint8_t game_done = 0;

// move counters
static uint32_t moves_level = 0;
static uint32_t moves_total = 0;
static uint32_t last_move_time = 0;

// timer values
static uint32_t level_start = 0;
static uint8_t time_left = 60;

// coin values
static uint8_t coins_level_total = 0;
static uint8_t coins_level_got = 0;
static uint16_t coins_total = 0;

// best score values
static uint32_t best_moves = 0;
static uint8_t best_set = 0;

// draws the dotted background
static void Draw_Background(void)
{
    LCD_Fill_Buffer(0);

    for (int y = 0; y < 240; y += 16) {
        for (int x = 0; x < 240; x += 16) {
            if (((x / 16) + (y / 16)) % 2 == 0) {
                LCD_Draw_Rect(x + 3, y + 3, 2, 2, 13, 1);
            }
        }
    }
}

// updates LED brightness depending on level and time left
static void Update_LED(void)
{
    if (game_done) {
        PWM_SetDuty(&pwm_cfg, 100);
        return;
    }

    if (time_left <= LOW_TIME) {
        if (((HAL_GetTick() / 200) % 2) == 0) {
            PWM_SetDuty(&pwm_cfg, led_level[level_num]);
        } else {
            PWM_SetDuty(&pwm_cfg, 0);
        }
    } else {
        PWM_SetDuty(&pwm_cfg, led_level[level_num]);
    }
}

// copies the current level into the working maze
// also counts how many coins are in that level
static void Load_Level(void)
{
    coins_level_total = 0;
    coins_level_got = 0;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            maze_now[i][j] = mazes[level_num][i][j];

            if (mazes[level_num][i][j] == COIN) {
                coins_level_total++;
            }
        }
    }
}

// resets player position and level values
static void Reset_Player(void)
{
    Load_Level();
    player_r = 1;
    player_c = 1;
    moves_level = 0;
    last_move_time = 0;
    level_start = HAL_GetTick();
    time_left = level_time[level_num];
    Update_LED();
}

// fully restarts the game back to level 1
static void Reset_Game(void)
{
    level_num = 0;
    moves_total = 0;
    coins_total = 0;
    game_done = 0;
    Reset_Player();
}

// returns the value of a cell in the current maze
static uint8_t Get_Cell(int r, int c)
{
    return maze_now[r][c];
}

// checks if a cell is a wall
// also treats outside the maze like a wall
static uint8_t Is_Wall(int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) {
        return 1;
    }

    if (Get_Cell(r, c) == WALL) {
        return 1;
    }

    return 0;
}

// sound when hitting a wall
static void Wall_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 400, 50);
    HAL_Delay(35);
    buzzer_tone(&buzzer_cfg, 550, 50);
    HAL_Delay(35);
    buzzer_tone(&buzzer_cfg, 700, 45);
    HAL_Delay(45);
    buzzer_off(&buzzer_cfg);
}

// sound for collecting a coin
static void Coin_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 1400, 25);
    HAL_Delay(30);
    buzzer_tone(&buzzer_cfg, 1800, 25);
    HAL_Delay(35);
    buzzer_off(&buzzer_cfg);
}

// sound for finishing one level
static void Level_Clear_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 900, 30);
    HAL_Delay(60);
    buzzer_tone(&buzzer_cfg, 1200, 30);
    HAL_Delay(70);
    buzzer_off(&buzzer_cfg);
}

// sound for finishing the full game
static void Win_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 900, 35);
    HAL_Delay(80);
    buzzer_tone(&buzzer_cfg, 1200, 35);
    HAL_Delay(80);
    buzzer_tone(&buzzer_cfg, 1600, 35);
    HAL_Delay(120);
    buzzer_off(&buzzer_cfg);
}

// sound for losing on time
static void Lose_Sound(void)
{
    buzzer_tone(&buzzer_cfg, 950, 90);
    HAL_Delay(500);
    buzzer_off(&buzzer_cfg);
}

// shows level clear message
static void Show_Level_Clear(void)
{
    char txt[32];

    Draw_Background();
    LCD_printString("LEVEL CLEAR!", 20, 65, 3, 3);

    sprintf(txt, "Next Level: %d", level_num + 2);
    LCD_printString(txt, 48, 120, 1, 2);

    LCD_Refresh(&cfg0);
    HAL_Delay(700);
}

// shows fail message when timer runs out
static void Show_Time_Up(void)
{
    Draw_Background();
    LCD_printString("TIME UP!", 45, 65, 2, 3);
    LCD_printString("Back to Level 1", 28, 120, 1, 2);
    LCD_Refresh(&cfg0);
    HAL_Delay(900);
}

// checks if player is standing on a coin
// if yes, remove it and update counters
static void Collect_Coin(void)
{
    if (Get_Cell(player_r, player_c) == COIN) {
        maze_now[player_r][player_c] = EMPTY;
        coins_level_got++;
        coins_total++;
        Coin_Sound();
    }
}

// tries to move the player by one cell
static void Try_Move(int dr, int dc)
{
    int new_r = player_r + dr;
    int new_c = player_c + dc;

    if (!Is_Wall(new_r, new_c)) {
        player_r = new_r;
        player_c = new_c;
        moves_level++;
        moves_total++;

        Collect_Coin();

        if (Get_Cell(player_r, player_c) == EXIT) {
            if (level_num < LEVELS - 1) {
                Level_Clear_Sound();
                Show_Level_Clear();
                level_num++;
                Reset_Player();
            } else {
                game_done = 1;

                if (!best_set || moves_total < best_moves) {
                    best_moves = moves_total;
                    best_set = 1;
                }

                Win_Sound();
                Update_LED();
            }
        }
    } else {
        Wall_Sound();
    }
}

// updates countdown timer for current level
static void Update_Timer(void)
{
    uint32_t passed_ms;
    uint32_t passed_s;
    uint8_t limit;

    if (game_done) {
        return;
    }

    passed_ms = HAL_GetTick() - level_start;
    passed_s = passed_ms / 1000;
    limit = level_time[level_num];

    if (passed_s >= limit) {
        time_left = 0;
        Update_LED();
        Lose_Sound();
        Show_Time_Up();
        Reset_Game();
        return;
    }

    time_left = limit - passed_s;
}

// handles joystick movement and updates game logic
static void Maze_Update(void)
{
    uint32_t now;
    UserInput joy;

    if (game_done) {
        Update_LED();
        return;
    }

    Update_Timer();
    Update_LED();

    now = HAL_GetTick();

    if ((now - last_move_time) < MOVE_DELAY) {
        return;
    }

    Joystick_Read(&joystick_cfg, &joystick_data);
    joy = Joystick_GetInput(&joystick_data);

    switch (joy.direction) {
        case N:
            Try_Move(-1, 0);
            last_move_time = now;
            break;
        case S:
            Try_Move(1, 0);
            last_move_time = now;
            break;
        case E:
            Try_Move(0, 1);
            last_move_time = now;
            break;
        case W:
            Try_Move(0, -1);
            last_move_time = now;
            break;
        default:
            break;
    }
}

// draws everything on the screen
static void Maze_Draw(void)
{
    Draw_Background();

    // final win screen
    if (game_done) {
        char txt1[32];
        char txt2[32];
        char txt3[32];

        LCD_printString("ALL 5 LEVELS", 18, 35, 3, 3);
        LCD_printString("CLEARED!", 42, 72, 3, 3);

        sprintf(txt1, "Your Moves: %lu", (unsigned long)moves_total);
        LCD_printString(txt1, 34, 120, 1, 2);

        sprintf(txt2, "Coins: %u", coins_total);
        LCD_printString(txt2, 62, 145, 6, 2);

        if (best_set) {
            sprintf(txt3, "High Score: %lu", (unsigned long)best_moves);
            LCD_printString(txt3, 28, 170, 1, 1);
        }

        LCD_printString("Press BT3 for Menu", 42, 205, 1, 1);
        LCD_Refresh(&cfg0);
        return;
    }

    // top bar
    LCD_Draw_Rect(0, 0, 240, 18, 9, 1);

    {
        char title[32];
        sprintf(title, "LEVEL %d OF %d", level_num + 1, LEVELS);
        LCD_printString(title, 28, 4, 1, 2);
    }

    // draw maze cells
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            uint16_t x = START_X + c * SIZE;
            uint16_t y = START_Y + r * SIZE;

            if (Get_Cell(r, c) == WALL) {
                LCD_Draw_Rect(x, y, SIZE, SIZE, 7, 1);
                LCD_Draw_Rect(x + 1, y + 1, SIZE - 2, SIZE - 2, 8, 0);
            } else if (Get_Cell(r, c) == EXIT) {
                LCD_Draw_Rect(x, y, SIZE, SIZE, 3, 1);
                LCD_Draw_Rect(x + 2, y + 2, SIZE - 4, SIZE - 4, 6, 0);
                LCD_Draw_Rect(x + 4, y + 4, SIZE - 8, SIZE - 8, 1, 0);
            } else if (Get_Cell(r, c) == COIN) {
                LCD_Draw_Rect(x, y, SIZE, SIZE, 13, 0);
                LCD_Draw_Rect(x + 4, y + 4, SIZE - 8, SIZE - 8, 6, 1);
                LCD_Draw_Rect(x + 5, y + 5, SIZE - 10, SIZE - 10, 10, 0);
            } else {
                LCD_Draw_Rect(x, y, SIZE, SIZE, 13, 0);
            }
        }
    }

    // draw player
    {
        uint16_t px = START_X + player_c * SIZE + 3;
        uint16_t py = START_Y + player_r * SIZE + 3;
        LCD_Draw_Rect(px, py, SIZE - 6, SIZE - 6, 6, 1);
    }

    // text at bottom
    {
        char t1[32];
        char t2[32];
        char t3[32];
        char t4[32];

        sprintf(t1, "L:%lu", (unsigned long)moves_level);
        sprintf(t2, "T:%lu", (unsigned long)moves_total);
        sprintf(t3, "%us", time_left);
        sprintf(t4, "C:%u/%u", coins_level_got, coins_level_total);

        LCD_printString(t1, 4, 208, 1, 1);
        LCD_printString(t2, 52, 208, 1, 1);
        LCD_printString(t4, 100, 208, 6, 1);
        LCD_printString(t3, 162, 208, 2, 1);
        LCD_printString("BT3", 202, 208, 1, 1);
    }

    LCD_Refresh(&cfg0);
}

// main function for game 1
MenuState Game1_Run(void)
{
    MenuState next_state = MENU_STATE_HOME;

    Reset_Game();

    // small start sound
    buzzer_tone(&buzzer_cfg, 1000, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);

    while (1) {
        uint32_t start_time = HAL_GetTick();
        uint32_t used_time;

        Input_Read();

        // BT3 exits game
        if (current_input.btn3_pressed) {
            PWM_SetDuty(&pwm_cfg, 0);
            next_state = MENU_STATE_HOME;
            break;
        }

        Maze_Update();
        Maze_Draw();

        // keeps frame rate steady
        used_time = HAL_GetTick() - start_time;
        if (used_time < FRAME_TIME) {
            HAL_Delay(FRAME_TIME - used_time);
        }
    }

    return next_state;
}
