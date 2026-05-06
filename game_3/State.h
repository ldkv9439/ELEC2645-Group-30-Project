/**
 * @file state.h
 * @brief Simple game state controller
 */

#ifndef STATE_H
#define STATE_H

#include "Joystick.h"

#include <stdint.h>

// ===== CONSTANTS =====

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// ===== DATA STRUCTURES =====

/**
 * @enum GameState_t
 * @brief Set up game states
 */
typedef enum {
    GAME_START_PAGE = 0,    // Waiting for player to start
    GAME_INSTRUCTION_PAGE,  // Display instructions
    GAME_LEVEL_PAGE,        // Display current level
    GAME_PLAYING,           // Playing mode state         
    GAME_WIN,               // Winning mode state
    GAME_PAUSE,
    GAME_LOSE_PAGE,         // Display game over
    GAME_REWARD_PAGE,       // Display reward given
    GAME_OVER               // Game over state
} GameState_t;

// ===== FUNCTIONS =====

void Game3_Update (Joystick_t* joy);
void Game_3_Render (void);

const char* get_game_state_name(GameState_t state);

extern void opening_page (void);
extern GameState_t game_state; 

#endif