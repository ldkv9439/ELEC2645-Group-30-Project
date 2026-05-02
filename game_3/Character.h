/**
 * @file Character.h
 * @brief Simple sprite controller with directional movement and state
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include "stm32l4xx_hal.h"
#include "tim.h"  
#include "Buzzer.h"
#include "PWM.h"    // For PWM control of the LED 
#include "Joystick.h"
#include "LCD.h"

#include <stdint.h>

// ===== CHARACTER STATES =====

/**
 * @enum CharacterState_t
 * @brief Set up character movement states
 */
typedef enum {
    CHAR_IDLE = 0,      // Not moving
    CHAR_WALKING,       // Moving in a direction
    CHAR_DASHING        // Fast movement (button triggered)
} CharacterState_t;

/**
 * @enum GameState_t
 * @brief Set up game states
 */
typedef enum {
    GAME_START_PAGE = 0,    // Waiting for player to start
    GAME_LEVEL_PAGE,        // Display current level
    GAME_PLAYING,           // Playing mode state         
    GAME_WIN,               // Winning mode state
    GAME_PAUSE,
    GAME_LOSE_PAGE,         // Display game over
    GAME_REWARD_PAGE,       // Display reward given
    GAME_OVER               // Game over state
} GameState_t;

// ===== CHARACTER DATA =====

/**
 * @struct Character_t
 * @brief Sprite position and state
 * 
 * Minimal structure: just the data needed
 * - Position on screen
 * - Current state (IDLE, WALKING, DASHING)
 * - Animation frame (for walking animation)
 * - Frame counter 
 * - Dash counter
 * - Collision counter
 * - Shield counter
 * - Dash charge and recharge
 */
typedef struct {
    int16_t x;                      // X position
    int16_t y;                      // Y position
    CharacterState_t state;         // Current state
    uint8_t animation_frame;        // 0 or 1 (walk cycle)
    uint8_t frame_counter;          // Counter for animation timing
    uint8_t dash_counter;           // Frames remaining in dash
    uint8_t collision_counter;      
    uint8_t shield_counter;
    uint8_t dash_charge;
    uint8_t dash_recharge;
    uint8_t sprite_rotation;
} Character_t;


// ===== CONSTANTS =====

#define CHAR_SPEED 7                // Pixels per frame (normal)
#define CHAR_DASH_SPEED 10           // Pixels per frame (dashing)
#define CHAR_DASH_DURATION 20       // Frames (dash lasts this long)
#define CHAR_COLLIDE_DURATION 40    // Frames (collision effect lasts this long)

// ===== FUNCTIONS =====

/**
 * @brief Initialize character at screen center
 */
void Character_Init(Character_t* character);

/**
 * @brief Update character position and state
 * 
 * Uses joy->direction for 8-way movement
 * Sets state to WALKING when moving, IDLE when stopped
 * Handles dash countdown and speed boost
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed);

/**
 * @brief Draw character sprite on LCD
 * 
 * Draws different sprite based on current state:
 * - IDLE: standing sprite
 * - WALKING: animated walk cycle
 * - DASHING: speed lines sprite
 */
void Character_Draw(Character_t* character);

void gameover_melody(void);
void start_melody (void);

const char* get_char_state_name(CharacterState_t state); 
const char* get_game_state_name(GameState_t state);

extern Character_t game_character;
extern void gameover_melody (void);
extern void start_melody (void);

#endif // CHARACTER_H
