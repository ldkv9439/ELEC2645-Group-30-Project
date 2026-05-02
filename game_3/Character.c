/**
 * @file coins.c
 * @brief Coin object implementation 
 */
 
#include "Character.h"
#include "stm32l4xx_hal.h"
#include "ghost.h"
#include "tim.h"  
#include "Buzzer.h"
#include "level.h"
#include "PWM.h"    // For PWM control of the LED 
#include "coins.h"
#include "ghost.h"

#include <stdint.h>


extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg;

// ===== ANIMATION SPRITES =====

/**
 * @brief IDLE animation - character stays static 
 * 16x16 pixel sprite showing idle
 */
const uint8_t CharacterIDLE[16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,5,5,5,255,255,255,255,255,255,255},
    {255,255,255,255,5,5,1,6,10,5,5,255,255,255,255,255},
    {255,255,255,5,1,1,6,6,6,6,10,5,255,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,5,6,255,6,6,6,255,6,6,10,5,255,255,255},
    {255,5,6,6,255,6,6,6,255,6,6,6,10,5,255,255},
    {255,5,7,7,255,5,5,6,255,7,7,6,10,5,255,255},
    {255,5,7,7,6,6,6,6,6,7,7,6,10,5,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,255,5,10,10,10,10,10,5,10,10,5,255,5,255},
    {255,255,255,5,10,6,6,6,10,5,5,10,10,5,5,255},
    {255,255,255,255,5,6,6,6,6,10,10,10,5,5,255,255},
    {255,255,255,255,255,5,12,5,5,5,12,5,255,255,255,255},
    {255,255,255,255,255,255,12,255,255,255,12,255,255,255,255,255},
    {255,255,255,255,255,12,12,12,255,12,12,12,255,255,255,255}
};

/**
 * @brief WALK LEFT animation
 * 16x16 pixel sprite showing character facing left side 
 */
const uint8_t CharacterWALKLEFT[16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,5,5,5,255,255,255,255,255,255,255},
    {255,255,255,255,5,5,1,6,10,5,5,255,255,255,255,255},
    {255,255,255,5,1,1,6,6,6,6,10,5,255,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,5,6,255,6,6,6,255,6,6,10,5,255,255,255},
    {255,5,6,6,255,6,6,6,255,6,6,6,10,5,255,255},
    {255,5,7,7,255,5,5,6,255,7,7,6,10,5,255,255},
    {255,5,7,7,6,6,6,6,6,7,7,6,10,5,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,255,5,10,10,10,10,10,5,10,10,5,255,255,5},
    {255,255,255,5,10,6,6,6,10,5,5,10,10,5,5,255},
    {255,255,255,255,5,6,6,6,6,10,10,10,5,5,255,255},
    {255,255,255,255,255,5,12,5,5,5,12,5,255,255,255,255},
    {255,255,255,255,255,255,12,255,255,255,12,255,255,255,255,255},
};

/**
 * @brief WALK RIGHT first animation
 * 16x16 pixel sprite showing character facing right side 
 */
const uint8_t CharacterWALKRIGHT_1[16][16] = {
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,5,5,5,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,5,5,5,255,255,255,255,255,255},
    {255,255,255,255,5,10,6,6,6,6,1,1,5,255,255,255},
    {255,255,255,5,10,6,6,6,6,6,6,6,6,5,255,255},
    {255,255,255,5,10,6,6,255,6,6,6,255,6,5,255,255},
    {255,255,5,10,6,6,6,255,6,6,6,255,6,6,5,255},
    {255,255,5,10,6,7,7,255,6,5,5,255,7,7,5,255},
    {255,255,5,10,6,7,7,6,6,6,6,6,7,7,5,255},
    {255,255,255,5,10,6,6,6,6,6,6,6,6,5,255,255},
    {5,255,255,5,10,10,5,10,10,10,10,10,5,255,255,255},
    {255,5,5,10,10,5,5,10,6,6,6,10,5,255,255,255},
    {255,255,5,5,10,10,10,6,6,6,6,5,255,255,255,255},
    {255,255,255,255,5,12,5,5,5,12,5,255,255,255,255,255},
    {255,255,255,255,255,12,255,255,255,12,255,255,255,255,255,255},
    {255,255,255,255,255,12,12,12,255,12,12,12,255,255,255,255}
};

/**
 * @brief WALK RIGHT second animation
 * 16x16 pixel sprite showing character facing right side 
 */
const uint8_t CharacterWALKRIGHT_2[16][16] = {
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,5,5,5,255,255,255,255,255,255},
    {255,255,255,255,255,5,5,10,6,1,5,5,255,255,255,255},
    {255,255,255,255,5,10,6,6,6,6,1,1,5,255,255,255},
    {255,255,255,5,10,6,6,6,6,6,6,6,6,5,255,255},
    {255,255,255,5,10,6,6,255,6,6,6,255,6,5,255,255},
    {255,255,5,10,6,6,6,255,6,6,6,255,6,6,5,255},
    {255,255,5,10,6,7,7,255,6,5,5,255,7,7,5,255},
    {255,255,5,10,6,7,7,6,6,6,6,6,7,7,5,255},
    {255,255,255,5,10,6,6,6,6,6,6,6,6,5,255,255},
    {255,5,255,5,10,10,5,10,10,10,10,10,5,255,255,255},
    {255,5,5,10,10,5,5,10,6,6,6,10,5,255,255,255},
    {255,255,5,5,10,10,10,6,6,6,6,5,255,255,255,255},
    {255,255,255,255,5,12,5,5,5,12,5,255,255,255,255,255},
    {255,255,255,255,255,12,255,255,255,12,255,255,255,255,255,255},
};

/**
 * @brief WALK UP & DOWN animation
 * 16x16 pixel sprite showing character moving up and down
 */
const uint8_t CharacterUPDOWN[16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,5,5,5,255,255,255,255,255,255,255},
    {255,255,255,255,5,5,1,6,10,5,5,255,255,255,255,255},
    {255,255,255,5,1,1,6,6,6,6,10,5,255,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,5,6,6,255,6,6,6,255,6,6,6,10,5,255,255},
    {255,5,7,7,255,5,5,6,255,7,7,6,10,5,255,255},
    {255,5,7,7,6,6,6,6,6,7,7,6,10,5,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,255,5,10,10,10,10,10,5,10,10,5,255,5,255},
    {255,255,255,5,10,6,6,6,10,5,5,10,10,5,5,255},
    {255,255,255,255,5,6,6,6,6,10,10,10,5,5,255,255},
    {255,255,255,255,255,5,12,5,5,5,12,5,255,255,255,255},
    {255,255,255,255,255,255,12,255,255,255,12,255,255,255,255,255},
};

/**
 * @brief DASHING animation - Speed lines around character
 * 16x16 pixel sprite showing dashing/moving fast
 */
const uint8_t CharacterDASHING[16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,6,255,255,255},
    {255, 6, 255, 255, 255, 255, 255, 255,255,255,255,6,255,255,255,255},
    {255,255,6,255,255,255,5,5,5,255,255,255,255,255,6,255},
    {6,255,255,255,5,5,1,6,10,5,5,255,255,255,255,255},
    {255,6,255,5,1,1,6,6,6,6,10,5,255,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,5,6,255,6,6,6,255,6,6,10,5,255,255,255},
    {255,5,6,255,255,255,6,255,255,255,6,6,10,5,255,255},
    {255,5,7,7,255,5,5,6,255,7,7,6,10,5,255,255},
    {255,5,7,7,6,6,6,6,6,7,7,6,10,5,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,255,5,10,10,10,10,10,5,10,10,5,255,5,255},
    {255,255,255,5,10,6,6,6,10,5,5,10,10,5,5,255},
    {255,255,255,255,5,6,6,6,6,10,10,10,5,5,255,255},
    {255,255,255,255,255,5,12,5,5,5,12,5,255,255,255,255},
    {255,255,255,255,255,255,12,255,255,255,12,255,255,255,255,255},
};


// ===== STATIC SPRITES =====

/**
 * @brief Chick Sprite in GAME_START_PAGE
 * 16x16 pixel sprite showing a chick
 */
const uint8_t CharacterMAINPAGE[16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,6,255,255,255},
    {255, 6, 255, 255, 255, 255, 255, 255,255,255,255,6,255,255,255,255},
    {255,255,6,255,255,255,5,5,5,255,255,255,255,255,6,255},
    {6,255,255,255,5,5,1,6,10,5,5,255,255,6,255,255},
    {255,6,255,5,1,1,6,6,6,6,10,5,255,255,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,5,6,255,6,6,6,255,6,6,10,5,255,255,255},
    {255,5,6,255,6,255,6,255,6,255,6,6,10,5,255,255},
    {255,5,7,7,6,5,5,6,6,7,7,6,10,5,255,255},
    {255,5,7,7,6,6,6,6,6,7,7,6,10,5,255,255},
    {255,255,5,6,6,6,6,6,6,6,6,10,5,255,255,255},
    {255,255,255,5,10,10,10,10,10,5,10,10,5,255,5,255},
    {255,255,255,5,10,6,6,6,10,5,5,10,10,5,5,255},
    {255,255,255,255,5,6,6,6,6,10,10,10,5,5,255,255},
    {255,255,255,255,255,5,12,5,5,5,12,5,255,255,255,255},
    {255,255,255,255,255,255,12,255,255,255,12,255,255,255,255,255}
};

/**
 * @brief Egg Sprite in GAME_WIN
 * 16x16 pixel sprite showing a fried chicken
 */
const uint8_t CharacterWINPAGE [16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,255,255,12,12,12,12,12,255,255,255},
    {255,255,255,255,255,255,255,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,255,255,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,255,12,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,255,12,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,12,12,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,12,12,12,12,12,12,12,12,12,255,255},
    {255,255,255,255,255,12,12,12,12,12,12,12,12,255,255,255},
    {255,255,255,255,255,12,12,12,12,12,12,12,255,255,255,255},
    {255,255,255,255,1,1,12,12,12,12,255,255,255,255,255,255},
    {255,255,255,1,1,1,255, 255,255,255,255,255,255,255,255,255},
    {255,1,1,1,1,255, 255, 255,255,255,255,255,255,255,255,255},
    {255,1,1,1,255,255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,1,1,255,255, 255, 255,255,255,255,255,255,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255}
};

/**
 * @brief Egg Sprite in GAME_OVER
 * 16x16 pixel sprite showing an egg
 */
const uint8_t CharacterLOSEPAGE [16][16] = {
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255,255,255,255,255,255,1,1,1,1,1,255,255,255,255,255},
    {255,255,255,255,1,1,1,1,1,1,1,1,255,255,255,255},
    {255,255,255,1,1,1,1,10,6,6,1,1,1,255,255,255},
    {255,255,1,1,1,1,10,10,6,6,1,1,1,1,255,255},
    {255,255,1,1,1,10,255,255,6,255,255,6,1,1,1,255},
    {255,1,1,1,1,10,10,255,10,10,255,10,1,1,1,255},
    {255,1,1,1,1,10,10,10,10,10,10,10,1,1,1,255},
    {255,255,1,1,1,1,10,10,10,10,10,1,1,1,1,255},
    {255,255,255,1,1,1,1,1,1,1,1,1,1,1,255,255},
    {255,255,255,255,255,1,1,1,1,1,1,1,255,255,255,255},
    {255,255,255,255,255,1,1,1,255,1,1,1,255,255,255,255},
    {255,255,255,255,1,1,1,1,255,1,1,1,1,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255},
    {255, 255, 255, 255, 255, 255, 255, 255,255,255,255,255,255,255,255,255}
};

void gameover_melody(void)
{
    printf("\nPlaying Pacman melody...\n");

    uint16_t melody[] = {
        NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5,
        NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_C5,
        NOTE_C6, NOTE_G6, NOTE_E6, NOTE_C6, NOTE_G6, NOTE_E6
    };

    // Note durations: 4 = quarter, 8 = eighth, 1 = whole (note duration is base_duration / noteDurations[i])
    uint8_t noteDurations[] = {
        16, 16, 16, 16, 
        32, 16, 8, 16, 
        16, 16, 16, 32, 16, 8
    };

    uint16_t tempo = 105;
    uint16_t wholenote = (60000 * 4) / tempo; // delay for a quarter note in ms

    uint16_t totalNotes = sizeof(noteDurations) / sizeof(noteDurations[0]);
    
    // The loop now runs for 10 notes
    for (int iNote = 0; iNote < totalNotes; iNote++) {
      
        uint16_t duration;

        if (noteDurations[iNote] > 0) {
            duration = wholenote / noteDurations[iNote];
        } else {
            duration = (wholenote / (-noteDurations[iNote])) * 1.5; // Default to quarter note duration for rests
        }

        buzzer_note(&buzzer_cfg, melody[iNote], 50); // Play the note at 50% duty cycle
        HAL_Delay(duration*0.9);
        
        buzzer_off(&buzzer_cfg);
        HAL_Delay(duration*0.1); 
    }
}

void start_melody (void) {
    // Play notes with sharps (chromatic scale from C4 to C5)
    int chromatic[] = {
        NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4,
        NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4, NOTE_C5
    };

    for (int i = 0; i < 13; i++) {
        buzzer_note(&buzzer_cfg, chromatic[i], 50);
        HAL_Delay(200);
    }

    buzzer_off(&buzzer_cfg);
}

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

/**
 * @brief Get game state name
 */
const char* get_game_state_name(GameState_t state) {
    switch (state) {
        case GAME_START_PAGE:  return "START";
        case GAME_LEVEL_PAGE:  return "LEVEL";
        case GAME_PLAYING:     return "PLAYING";
        case GAME_WIN:         return "WIN";
        case GAME_PAUSE:       return "PAUSE";
        case GAME_LOSE_PAGE:   return "LOSE";
        case GAME_REWARD_PAGE: return "REWARD";
        case GAME_OVER:        return "OVER";
        default:               return "???";
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
    
    // Handle red and yellow LED blinking when dashing / collision with ghost
    uint32_t current_time = HAL_GetTick();

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
    if (new_x < 25) new_x = 25;
    if (new_x > 215) new_x = 215;
    if (new_y < 70) new_y = 70;
    if (new_y > 215) new_y = 215;

    // Handle collision betwen the character and the ghosts
    int ghost_radius = 9;
    int character_radius = 10;

    if (level_state == BOSSLEVEL) {
        ghost_radius = 27;
    }

    if (character->collision_counter == 0){
        for (int i=0; i < ghost_count; i++){
            if (Circle_Overlap(new_x, new_y, character_radius, ghosts[i].x, ghosts[i].y, ghost_radius)) {
                character->collision_counter = CHAR_COLLIDE_DURATION;
                new_x = character->x + (move_x * 1);
                new_y = character->y + (move_y * 1);
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
    
    int16_t x_pos = character->x - 16;  
    int16_t y_pos = character->y - 16;
    
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