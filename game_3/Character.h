/**
 * @file Character.h
 * @brief Simple sprite controller with directional movement and state
 */

#ifndef CHARACTER_H
#define CHARACTER_H

#include "Joystick.h"

#include <stdint.h>

// ===== CONSTANTS =====

// ---- Boundaries ----
#define CHAR_MIN_X 25      // Minimum x-axis boundary
#define CHAR_MAX_X 215     // Maximum x-axis boundary
#define CHAR_MIN_Y 70      // Minimum y-axis boundary
#define CHAR_MAX_Y 215     // Maximum y-axis boundary

// ---- Speed ----
#define CHAR_SPEED 7                // Pixels per frame (normal)
#define CHAR_DASH_SPEED 10          // Pixels per frame (dashing)

// ---- Duration ----
#define CHAR_DASH_DURATION 20       // Frames (dash lasts this long)
#define CHAR_COLLIDE_DURATION 20    // Frames (collision effect lasts this long)

// ---- Size ----
#define CHAR_COLLISION_RADIUS 10    // Character radius when collide with ghost
#define CHAR_HALF 16                // Half of character size (16x16 scaled to 2)

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

// ===== CHARACTER DATA =====

/**
 * @struct Character_t
 * @brief Sprite position and state
 */
typedef struct {
    int16_t x;                      // X position 
    int16_t y;                      // Y position
    CharacterState_t state;         // Current FSM state
    uint8_t animation_frame;        // Current animation frame index (0 or 1) 
    uint8_t frame_counter;          // Counter to control animation speed
    uint8_t dash_counter;           // Frames remaining when dashing ( >0 means dashing)
    uint8_t collision_counter;      // Frames remaining after collide with ghost      
    uint8_t shield_counter;         // Timer for shield duration 
    uint8_t dash_charge;            // Number of dashes available
    uint8_t dash_recharge;          // Counter to recharge number of dashes
    uint8_t sprite_rotation;        // Character rotation
} Character_t;

// ===== FUNCTIONS =====

/**
 * @brief Initialize character to default state
 * - Spawns at bottom-center of the screen
 * - Sets FSM state to IDLE
 * - Resets all timers (dash, animation, collision)
 */
void Character_Init(Character_t* character);

/**
 * @brief Update character position and state
 * 
 * - Uses joy->direction for 8-way movement
 * - Sets state to WALKING when moving, IDLE when stopped
 * - Handles dash countdown and speed boost
 * - Handles collision with ghosts
 * - Manage LED and buzzer feedback
 *
 * @param character Pointer to character object
 * @param joy Current joystick input
 * @param dash_pressed Button input to activate dash
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed);

/**
 * @brief Draw character sprite on LCD
 * 
 * - Applies offset to character sprite to its center position
 * - Draws different sprite based on current state:
 * - IDLE: standing sprite
 * - WALKING: animated walk cycle
 * - DASHING: speed lines sprite
 */
void Character_Draw(Character_t* character);

const char* get_char_state_name(CharacterState_t state); 

extern Character_t game_character;

#endif
