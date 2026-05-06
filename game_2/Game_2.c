#include "Game_2.h"
#include "InputHandler.h"
#include "Menu.h"
#include "LCD.h"
#include "PWM.h"
#include "Buzzer.h"
#include "Joystick.h"
#include "PVZEngine.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

extern ST7789V2_cfg_t cfg0;
extern Buzzer_cfg_t buzzer_cfg;
extern Joystick_cfg_t joystick_cfg;
extern PWM_cfg_t pwm_cfg;

// ===== GAME 2 CONSTANTS =====
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  240
#define HUD_HEIGHT      24
#define GAME2_FRAME_TIME_MS 50  // ~20 FPS


/**
 * @brief Game 2 - Plants vs Zombies Implementation
 * 
 * This game runs within the Game_2_Run() function and returns control
 * to the menu system when the player exits.
 * 
 * All game state, rendering, and updates are self-contained here.
 */

// ===== GAME STATE =====
static PVZEngine_t pvz_engine;
static Joystick_t joystick_data;

// ===== HELPER FUNCTIONS =====

/**
 * @brief Draw the static background for PvZ
 */
void Game2_DrawBackground(void) {
    /* Set PvZ custom palette */
    LCD_Set_Palette(PALETTE_CUSTOM);
    
    /* --- SKY --- */
    LCD_Draw_Rect(0, HUD_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - HUD_HEIGHT, 14, 1);

    /* ===================== ROAD ===================== */
    LCD_Draw_Rect(200, HUD_HEIGHT, 40, SCREEN_HEIGHT - HUD_HEIGHT, 1, 1);
    LCD_Draw_Rect(193, HUD_HEIGHT, 8,  SCREEN_HEIGHT - HUD_HEIGHT, 2, 1);

    /* lane stripes */
    LCD_Draw_Rect(224, HUD_HEIGHT + 4,   7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 36,  7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 68,  7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 100, 7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 132, 7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 164, 7, 18, 2, 1);
    LCD_Draw_Rect(224, HUD_HEIGHT + 196, 7, 18, 2, 1);

    /* ===================== HOUSE ===================== */
    LCD_Draw_Rect(0, HUD_HEIGHT, 30, SCREEN_HEIGHT - HUD_HEIGHT, 2, 1);

    /* roof */
    LCD_Draw_Rect(0, HUD_HEIGHT,      34, 18, 3,  1);
    LCD_Draw_Rect(0, HUD_HEIGHT + 18, 34, 5,  4, 1);

    /* window */
    LCD_Draw_Rect(3, 75, 25, 20, 15, 1);
    LCD_Draw_Rect(3, 75, 25, 20, 5, 0);
    LCD_Draw_Line(15, 75, 15, 93, 5);
    LCD_Draw_Line(3, 84, 26, 84, 5);

    /* door */
    LCD_Draw_Rect(5, 108, 25, 48, 5, 1);
    LCD_Draw_Circle(26, 132, 2, 8, 1);

    /* plant pot */
    LCD_Draw_Circle(22, 165, 7, 10, 1);   
    LCD_Draw_Rect(16, 168, 13, 10, 5, 1);

    /* mailbox */
    LCD_Draw_Rect(13, HUD_HEIGHT + 164, 15, 10, 3, 1);
    LCD_Draw_Rect(20, HUD_HEIGHT + 174, 3, 17, 5, 1);

    /* ===================== STONE BORDER ===================== */
    LCD_Draw_Rect(GRID_ORIGIN_X - 7, GRID_ORIGIN_Y - 7, GRID_COLS * CELL_W + 14, 7,  1, 1);
    LCD_Draw_Rect(GRID_ORIGIN_X - 7, GRID_ORIGIN_Y + GRID_ROWS * CELL_H, GRID_COLS * CELL_W + 14, 7,  1, 1);
    LCD_Draw_Rect(GRID_ORIGIN_X - 7, GRID_ORIGIN_Y - 7, 7, GRID_ROWS * CELL_H + 14, 1, 1);
    LCD_Draw_Rect(GRID_ORIGIN_X + GRID_COLS * CELL_W, GRID_ORIGIN_Y - 7, 7, GRID_ROWS * CELL_H + 14, 1, 1);

    /* ===================== LAWN ===================== */
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            uint8_t c = ((row + col) % 2 == 0) ? 10 : 11;
            LCD_Draw_Rect(
                GRID_ORIGIN_X + col * CELL_W,
                GRID_ORIGIN_Y + row * CELL_H,
                CELL_W, CELL_H, c, 1
            );
        }
    }

    /* ===================== PICKET FENCE ===================== */
    LCD_Draw_Rect(GRID_ORIGIN_X, GRID_ORIGIN_Y - 8, GRID_COLS * CELL_W, 4, 2, 1);
    for (int i = 0; i * 9 < GRID_COLS * CELL_W; i++) {
        LCD_Draw_Rect(GRID_ORIGIN_X + i * 9, GRID_ORIGIN_Y - 14, 5, 11, 2, 1);
    }

    /* ===================== TREE ===================== */
    LCD_Draw_Circle(228, HUD_HEIGHT + 16, 10, 10, 1);
    LCD_Draw_Rect(225,   HUD_HEIGHT + 22,  5, 10,  5, 1);
}

/**
 * @brief Update game logic for one frame
 */
static void Game2_Update(UserInput input) {
    PVZEngine_Update(&pvz_engine, input);
}

/**
 * @brief Render game to LCD buffer
 */
static void Game2_Render(void) {
    LCD_Fill_Buffer(0);
    Game2_DrawBackground();
    PVZEngine_Draw(&pvz_engine);
    LCD_Refresh(&cfg0);
}

/**
 * @brief Convert joystick data to UserInput for PvZ engine
 */
static UserInput Game2_GetInput(void) {
    Joystick_Read(&joystick_cfg, &joystick_data);
    UserInput input = Joystick_GetInput(&joystick_data);
    return input;
}

// ===== MAIN GAME FUNCTION =====

/**
 * @brief Game 2 Run - Plants vs Zombies
 * 
 * Main entry point for Game 2. This function:
 * - Initializes the PvZ engine
 * - Runs the main game loop with proper frame timing
 * - Handles input and updates
 * - Returns when player exits (via menu button or game over)
 * 
 * @return MenuState - Where to go next (typically MENU_STATE_HOME)
 */
MenuState Game2_Run(void) {
    // ===== INITIALIZATION =====
    
    // Initialize PvZ engine
    PVZEngine_Init(&pvz_engine);
    printf("Game 2: PvZ engine initialised.\r\n");
    
    // Play startup sound
    buzzer_tone(&buzzer_cfg, 1200, 30);
    HAL_Delay(50);
    buzzer_off(&buzzer_cfg);
    
    // Draw static background
    Game2_DrawBackground();
    LCD_Refresh(&cfg0);
    HAL_Delay(500);
    
    MenuState exit_state = MENU_STATE_HOME;
    
    // ===== MAIN GAME LOOP =====
    uint32_t last_tick = HAL_GetTick();
    
    while (1) {
        uint32_t now = HAL_GetTick();
        
        // Frame timing - maintain consistent frame rate
        if ((now - last_tick) < GAME2_FRAME_TIME_MS) {
            continue;
        }
        last_tick = now;
        
        // ===== INPUT =====
        UserInput input = Game2_GetInput();

        Input_Read();

        // Check if button was pressed to return to menu
        if (current_input.btn1_pressed) {
            exit_state = MENU_STATE_HOME;
            break;  // Exit game loop
        }
        // ===== UPDATE =====
        Game2_Update(input);
        
        // ===== RENDER =====
        Game2_Render();
    }
    
    printf("Game 2: Exiting to menu.\r\n");
    return exit_state;
}
