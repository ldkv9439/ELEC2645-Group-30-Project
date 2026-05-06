/**
 * @file Character.c
 * @brief Character object implementation 
 */
 
#include "Character.h"
#include "coins.h"
#include "stm32l4xx_hal.h"
#include "Buzzer.h"
#include "PWM.h"  
#include "LCD.h"
#include "Level.h"
#include "Coins.h"
#include "Ghost.h"
#include "Sprites.h"

#include <limits.h>
#include <stdint.h>

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg;

/**
 * @brief Get character state name
 */
const char* get_char_state_name(CharacterState_t state) {
    switch (state) {
        case CHAR_IDLE:    return "IDLE";
        case CHAR_WALKING: return "WALK";
        case CHAR_DASHING: return "DASH";
        default:           return "???";
    }
}

// ===== IMPLEMENTATION =====

/**
 * Initialize character at middle bottom of screem, in idle mode, maximum dash count
 */
void Character_Init(Character_t* character) {
    character->x = 120;
    character->y = 215;
    character->state = CHAR_IDLE;
    character->animation_frame = 0;
    character->frame_counter = 0;
    character->dash_counter = 0;
    character->collision_counter = 0;
    character->dash_charge = 2;
    character->dash_recharge = 0;   
    character->sprite_rotation = S;
}

/**
 * Update character position and state
 * 
 * Movement: Use joystick->direction for 8-way movement
 * State: IDLE when stopped, WALKING when moving, DASHING on button press
 */
void Character_Update(Character_t* character, Joystick_t* joy, uint8_t dash_pressed) {
    
    // Initialize led blink time
    static uint32_t last_led_blink_time = 0;

    // Initialize led flag to set state of led
    static uint8_t led_on = 0;

    // Calculate movement based on joystick direction
    int16_t move_x = 0;
    int16_t move_y = 0;

    switch (joy->direction) {
        case N:  move_y = -1; character->sprite_rotation = N; break;
        case NE: move_x = 1; move_y = -1; character->sprite_rotation = NE; break;
        case E:  move_x = 1; character->sprite_rotation = E; break;
        case SE: move_x = 1; move_y = 1; character->sprite_rotation = SE; break;
        case S:  move_y = 1; character->sprite_rotation = S; break;
        case SW: move_x = -1; move_y = 1; character->sprite_rotation = SW; break;
        case W:  move_x = -1; character->sprite_rotation = W; break;
        case NW: move_x = -1; move_y = -1; character->sprite_rotation = NW; break;
        default: break;  // CENTRE - no movement
    }
    
    // Handle dash button 
    if (dash_pressed && character->dash_counter == 0 && character->dash_charge > 0) {
        character->dash_counter = CHAR_DASH_DURATION;
        character->dash_charge--;
        buzzer_note(&buzzer_cfg, NOTE_A6, 50);
    }
    
    // Handle red LED blinking when dashing / collision with ghost
    uint32_t current_time = HAL_GetTick();

    // Red LED blink fast when character dash
    if (character->dash_counter > 0) {
        if (current_time - last_led_blink_time >= 50){
            led_on = !led_on;

            if (led_on) {
                PWM_SetDuty(&pwm_cfg, 100);
            } else {
                PWM_SetDuty(&pwm_cfg, 0);
            }

            last_led_blink_time = current_time;
        }
    // Red LED blink slow when character collides with ghosts
    } else if (character->collision_counter > 0) {
        if (current_time - last_led_blink_time >= 500){
            led_on = !led_on;

            if (led_on) {
                PWM_SetDuty(&pwm_cfg, 100);
            } else {
                PWM_SetDuty(&pwm_cfg, 10);
            }

            last_led_blink_time = current_time;
        }
    }

    // Change states of buzzer and LED after dashing / collision with ghost
    if (character->dash_counter == 0 && character->collision_counter == 0) {
        buzzer_off(&buzzer_cfg);
        led_on = 1;
        PWM_SetDuty(&pwm_cfg, 100);
    }
    
    // Apply movement with speed (normal or dash)
    uint8_t current_speed = CHAR_SPEED;

    if (character->dash_counter > 0) {
        current_speed = CHAR_DASH_SPEED;
        character->dash_counter--;
    } else if (character->collision_counter > 0) {
        current_speed = 1;
        character->collision_counter--;
    }
    
    int16_t new_x = character->x + (move_x * current_speed);
    int16_t new_y = character->y + (move_y * current_speed);
    
    // Clamping character sprite inside game border
    if (new_x < CHAR_MIN_X) new_x = CHAR_MIN_X;
    if (new_x > CHAR_MAX_X) new_x = CHAR_MAX_X;
    if (new_y < CHAR_MIN_Y) new_y = CHAR_MIN_Y;
    if (new_y > CHAR_MAX_Y) new_y = CHAR_MAX_Y;

    // Handle collision betwen the character and the ghosts
    int ghost_radius = NORMAL_GHOST_COLLISION_RADIUS;
    int ghost_half = NORMAL_GHOST_HALF;

    if (level_state == BOSSLEVEL) {
        ghost_radius = BOSS_GHOST_COLLISION_RADIUS;
        ghost_half = BOSS_GHOST_HALF;
    }

    if (character->collision_counter == 0){
        for (int i=0; i < ghost_count; i++){

            // Find center of ghost
            int ghost_center_x = ghosts[i].x + ghost_half;
            int ghost_center_y = ghosts[i].y + ghost_half;

            // Change character speed to 1 (slower)
            if (Circle_Overlap(character->x, character->y, CHAR_COLLISION_RADIUS, ghost_center_x, ghost_center_y, ghost_radius)) {
                character->collision_counter = CHAR_COLLIDE_DURATION;
                new_x = character->x + (move_x * 1);
                new_y = character->y + (move_y * 1);
                break;
            }
        } 
    }

    if (move_x != 0 || move_y != 0) {
        character->x = new_x;
        character->y = new_y;
    }

    // Update state (IDLE, WALKING, DASHING) 
    uint8_t is_moving = (move_x != 0 || move_y != 0);
    
    if (character->dash_counter > 0) {
        character->state = CHAR_DASHING;
    } else if (is_moving) {
        character->state = CHAR_WALKING;
    } else {
        character->state = CHAR_IDLE;
    }
    
    // Update animation frame for walk cycle
    if (character->state == CHAR_WALKING) {
        character->frame_counter++;
        if (character->frame_counter >= 5) {
            character->frame_counter = 0;
            character->animation_frame = (character->animation_frame + 1) % 2;
        }
    } else {
        character->animation_frame = 0;
        character->frame_counter = 0;
    }

    // Recharge and Charge Dash 
    if (character->dash_charge < 2) {
        character->dash_recharge++;
        
        if (character->dash_recharge > 120) {
            character->dash_charge++;
            character->dash_recharge = 0;
        }
    }
}

/**
 * Draw character sprite based on current state
 */
void Character_Draw(Character_t* character) {
    
    int16_t x_pos = character->x - CHAR_HALF;  
    int16_t y_pos = character->y - CHAR_HALF;
    
    switch (character->state) {
        case CHAR_IDLE:
            LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterIDLE, 2);
            break;
        
        case CHAR_WALKING:
            // Walking animation if character is moving right
            if (character->sprite_rotation == E || character->sprite_rotation == NE || character->sprite_rotation == SE) {
                if (character->animation_frame == 0) {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterWALKRIGHT_1, 2);
                } else {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterWALKRIGHT_2, 2);
                }
            }

            // Walking animation if character is moving left
            else if (character->sprite_rotation == W || character->sprite_rotation == NW || character->sprite_rotation == SW) { 
                if (character->animation_frame == 0) {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterIDLE, 2);
                } else {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterWALKLEFT, 2);
                }
            }

            // Walking animation if character is moving up and down
            else { 
                if (character->animation_frame == 0) {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterUPDOWN, 2);
                } else {
                    LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterIDLE, 2);
                }
            }
            break;
        
        case CHAR_DASHING:
            LCD_Draw_Sprite_Scaled(x_pos, y_pos, 16, 16, (uint8_t*)CharacterDASHING, 2);
            break;
    }
}