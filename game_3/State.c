/**
 * @file state.c
 * @brief Game state implementation 
 */

#include "State.h"
#include "InputHandler.h"
#include "Joystick.h"   
#include "Buzzer.h"
#include "PWM.h"    
#include "LCD.h"        
#include "Level.h"
#include "Coins.h"
#include "Ghost.h"
#include "Flame.h"
#include "Character.h"
#include "Sprites.h"
#include "stm32l4xx_hal.h"

#include <stdint.h>
#include <stdlib.h>

// Initial flag state to see if a level is selected
static uint8_t status_level = 0; 

// Set all flags to play buzzer sound in different game states to 0 
static int start_melody_play = 0;
static int win_melody_play = 0;
static int gameover_melody_play = 0;

// Initialize timers
static uint32_t instruction_page_timer = 0;
static uint32_t level_page_timer = 0;
static uint32_t lose_page_timer = 0;
static uint32_t reward_page_timer = 0;

extern Buzzer_cfg_t buzzer_cfg;
extern PWM_cfg_t pwm_cfg;
extern PWM_cfg_t pwm_cfg_2;
extern Joystick_cfg_t joystick_cfg;
extern ST7789V2_cfg_t cfg0;

char score_str[24];

/**
 * @brief Get game state name
 */
const char* get_game_state_name(GameState_t state) {
    switch (state) {
        case GAME_START_PAGE:       return "START";
        case GAME_INSTRUCTION_PAGE: return "INSTRUCTIONS";
        case GAME_LEVEL_PAGE:       return "LEVEL";
        case GAME_PLAYING:          return "PLAYING";
        case GAME_WIN:              return "WIN";
        case GAME_PAUSE:            return "PAUSE";
        case GAME_LOSE_PAGE:        return "LOSE";
        case GAME_REWARD_PAGE:      return "REWARD";
        case GAME_OVER:             return "OVER";
        default:                    return "???";
    }
}

/**
 * @brief Update character logic (FSM, movement)
 * 
 * This function handles all game logic updates:
 * - Processes input (joystick and button)
 * - Updates character FSM state transitions
 * - Updates animation frames
 */
void update_character(Joystick_t* joy) {
    // Check if dash was pressed and clear the flag
    uint8_t dash_pressed = current_input.btn2_pressed;
    
    // Update character FSM with current input
    Character_Update(&game_character, joy, dash_pressed);
}

// Buzzer sound when enters game over
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

// Buzzer sound when game starts
void start_melody (void) {
    // Play notes from C4 to C5
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

// Buzzer sound when enters game win
void win_melody (void) {
    buzzer_note(&buzzer_cfg, NOTE_C6, 40);
    HAL_Delay(100);

    buzzer_note(&buzzer_cfg, NOTE_E6, 40);
    HAL_Delay(100);

    buzzer_note(&buzzer_cfg, NOTE_G6, 40);
    HAL_Delay(100);

    buzzer_note(&buzzer_cfg, NOTE_C7, 50);
    HAL_Delay(500);
    
    buzzer_off(&buzzer_cfg);
}

void loading_line_animation(uint16_t x, uint16_t y) {
    // Initialize length of loading line
    static uint16_t len = 0;
    
    LCD_Draw_Rect(x, y, len, 5, 1, 1);

    // Limiting the length
    if (len < 130) {
        // Draw small white rectangles
        len += 5;
    }
}

void chick_intro_animation (uint16_t x, uint16_t y) {
    static uint8_t frame = 0;
    
    // Small character animation
    if (frame == 0) {
      LCD_Draw_Sprite_Scaled(x, y, 16, 16, (uint8_t*)CharacterWALKRIGHT_1, 2);
    } else {
      LCD_Draw_Sprite_Scaled(x, y, 16, 16, (uint8_t*)CharacterWALKRIGHT_2, 2);
    }

    frame = !frame;
}

void opening_page (void) {
    LCD_Fill_Buffer(0);

    LCD_printString("Chick", 50, 50, 1, 5);
    LCD_printString("Knight", 40, 110, 1, 5);

    loading_line_animation(40, 200);
    chick_intro_animation(180, 180);

    LCD_Refresh(&cfg0);

    HAL_Delay(80);  // controls animation speed
}

void draw_grid_background (void) {
    int box_size = 20;

    for (int x = 0; x < SCREEN_WIDTH; x += box_size) {
        for (int y = 0; y < SCREEN_HEIGHT; y += box_size) {
            
            int column = (x - SCREEN_WIDTH) / box_size;
            int rows = (y - SCREEN_HEIGHT) / box_size;

            uint8_t grid_colour = ((column + rows) % 2 == 0) ? 14:15; 

            int width = box_size;
            int height = box_size;

            if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
            if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;

            LCD_Draw_Rect(x, y, width, height, grid_colour, 1);
        }
    }
}

/**
 * @brief Render the game to the LCD screen
 * 
 * This function handles all rendering/drawing:
 * - Clears the screen buffer
 * - Draws character sprite
 * - Refreshes LCD to display the frame
 */
void render_game(void) {

    draw_grid_background();
    
    // Draw game border
    LCD_Draw_Rect(0, 0, SCREEN_WIDTH, 7, 13, 1);
    LCD_Draw_Rect(0, SCREEN_HEIGHT - 7, SCREEN_WIDTH, 7, 13, 1);
    LCD_Draw_Rect(0, 0, 7, SCREEN_HEIGHT, 13, 1);
    LCD_Draw_Rect(SCREEN_WIDTH - 7, 0, 7, SCREEN_HEIGHT, 13, 1);

    // Draw flame at current position with animation
    Flame_Draw();
    // Draw coins at set position
    Coins_Draw();  
    // Draw ghost sprite shooting bullets at set position
    Ghost_Bullet_Draw();
    // Draw character at current position with animation
    Character_Draw(&game_character);

    // Print score board
    sprintf(score_str, "Score:%d", score); 
    LCD_printString(score_str, 15, 17, 1, 2);

    // Print dash availibility and usage
    char dash_str[24];
    sprintf(dash_str, "Dash:%d", game_character.dash_charge);
    LCD_printString(dash_str, 140, 17, 1, 2);

    // Print life count 
    char life_str[24];
    sprintf(life_str, "Life:%d", life); 
    LCD_printString(life_str, 140, 34, 3, 2);

    // Print shield count
    char shield_str[24];
    sprintf(shield_str, "Shield:%d", shield); 
    LCD_printString(shield_str, 15, 34, 4, 2);
}


void Game3_Update (Joystick_t* joy) {

    PWM_SetDuty(&pwm_cfg, 0);
    PWM_SetDuty(&pwm_cfg_2, 0);

    if(game_state != GAME_PLAYING) {
        LCD_Fill_Buffer(0);
    }

    switch (game_state) {

        case GAME_START_PAGE:
            LCD_printString("Chick", 60, 20, 10, 4);
            LCD_printString("Knight", 50, 60, 10, 4);
            LCD_Draw_Sprite_Scaled(80, 100, 16, 16, (uint8_t *)CharacterSTARTPAGE, 5);
            LCD_printString("Press Joystick Btn", 15, 200, 1, 2);
            LCD_printString("To Start", 80, 220, 1, 2);

            LCD_Refresh(&cfg0);

            if (start_melody_play == 0) {
                start_melody();         // Play starting melody on buzzer
                start_melody_play = 1;  // Change flag to 1
            }

            if (current_input.btn3_pressed) {
                current_input.btn3_pressed = 0; // Clear flag

                level_state = LEVEL1;         // Set level to Level 1
                status_level = 0;             // No level selected
                game_state = GAME_INSTRUCTION_PAGE; // Move to level display page

                start_melody_play = 0;        // Clear flag
            }
            break;
        
        case GAME_INSTRUCTION_PAGE:         
            if (instruction_page_timer == 0) {
                instruction_page_timer = HAL_GetTick();
            }

            LCD_printString("INSTRUCTIONS:", 10, 20, 10, 3);
            LCD_printString("Collect all coins", 10, 60, 1, 2);
            LCD_printString("per level", 10, 75, 1, 2);

            LCD_printString("Avoid flame,bullets", 10, 105, 1, 2);
            LCD_printString("and ghosts", 10, 120, 1, 2);

            LCD_printString("Hits reduce shield", 10, 150, 1, 2);
            LCD_printString("and life", 10, 165, 1, 2);

            LCD_printString("Shield and dash", 10, 195, 1, 2);
            LCD_printString("will recharge", 10, 210, 1, 2);

            LCD_Refresh(&cfg0);

            if (HAL_GetTick() - instruction_page_timer >= 2500) {
                instruction_page_timer = 0;
                game_state = GAME_LEVEL_PAGE;
            }
            break;

        case GAME_LEVEL_PAGE:
            if (level_page_timer == 0) {
                level_page_timer = HAL_GetTick();
            }

            if (level_state == LEVEL1) {
                LCD_printString("LEVEL",  40, 70, 1, 6);
                LCD_printString("1", 100, 140, 1,6);             
            } else if (level_state == LEVEL2) {
                LCD_printString("LEVEL",  40, 70, 1, 6);
                LCD_printString("2", 100, 140, 1,6);
            } else if (level_state == LEVEL3) {
                LCD_printString("LEVEL",  40, 70, 1, 6);
                LCD_printString("3", 100, 140, 1,6);
            } else if (level_state == BOSSLEVEL) {
                LCD_printString("BOSS",  45, 70, 1, 6);
                LCD_printString("LEVEL", 40, 140, 1,6);
            }

            LCD_Refresh(&cfg0);
                        
            Character_Init(&game_character);  // Initialize all elements in the character

            buzzer_off(&buzzer_cfg);          // Turn off buzzer

            // Determine next game state based on levels
            if (HAL_GetTick() - level_page_timer >= 2000) {
                level_page_timer = 0;

                if (level_state == BOSSLEVEL) {
                    game_state = GAME_REWARD_PAGE;
                } else {
                    game_state = GAME_PLAYING;
                }
            }
            break;
        
        case GAME_PLAYING:
            PWM_SetDuty(&pwm_cfg_2, 100);

            // Check if no level has been selected yet, switch between levels
            if (!status_level) {
                switch (level_state) {
                case LEVEL1:
                    Level1();
                    break;
                case LEVEL2:
                    Level2();
                    break;
                case LEVEL3:
                    Level3();
                    break;
                case BOSSLEVEL:
                    BossLevel();
                    break;
                }
            
                status_level = 1;

            }

            if (current_input.btn4_pressed) {
                current_input.btn4_pressed = 0;
                game_state = GAME_PAUSE;
                break;
            }

            update_character(joy);  
            Coins_Update(&game_character);
            Ghost_Update(&game_character);
            Flame_Update(&game_character);
            render_game(); 
            LCD_Refresh(&cfg0);

            // Switch to game lose page if character's life is zero
            if (life <= 0 && game_state != GAME_LOSE_PAGE) {
                life = 0;
                game_state = GAME_LOSE_PAGE;
            } 
            
            // Character proceed to the next level if all coins have been collected in that level
            // Switch to game win page if managed to pass Boss Level
            if (coins_remaining == 0) {
                if (level_state == BOSSLEVEL) {
                    game_state = GAME_WIN;
                } else {
                    status_level = 0;
                    level_state++;
                    game_state = GAME_LEVEL_PAGE;
                    break;
                }
            }
            break;
        
        case GAME_WIN:
            LCD_printString("WINNERWINNER", 15, 30, 10, 3); 
            LCD_printString("CHICKENDINNER", 5, 70, 10, 3);
            LCD_Draw_Sprite_Scaled(30, 110, 16, 16, (uint8_t *)ChickenWINPAGE, 5);
            LCD_printString("SCORE:", 150, 120, 1, 2);
            sprintf(score_str, "%d", score);
            LCD_printString(score_str, 160, 150, 1, 3);
            LCD_printString("Press Joystick Btn", 15, 200, 1, 2);
            LCD_printString("To Restart", 60, 220, 1, 2);

            LCD_Refresh(&cfg0);

            if (win_melody_play == 0) {
                win_melody();         // Play win melody on buzzer
                win_melody_play = 1;  // Change flag to 1
            }

            // Restart game 
            if (current_input.btn3_pressed) {
                current_input.btn3_pressed = 0;    // Clear flag
                life = LIFE_MAX;      // Set character's life to maximum
                shield = SHIELD_MAX;  // Set character's shield to maximum    
                score = 0;            // Clear score
                Character_Init(&game_character); // Reset character

                level_state = LEVEL1; // Set level to Level 1
                status_level = 0;
                game_state = GAME_LEVEL_PAGE; // Move to level display page

                win_melody_play = 0;          // Clear flag
            }
            break;
        
        case GAME_PAUSE:
            LCD_printString("GAME", 50, 70, 1, 6); 
            LCD_printString("PAUSED", 20, 140, 1, 6); 

            LCD_Refresh(&cfg0);

            if (current_input.btn4_pressed) {
                current_input.btn4_pressed = 0;
                game_state = GAME_PLAYING;
            }
            break;

        case GAME_LOSE_PAGE:
            if (lose_page_timer == 0) {
                lose_page_timer = HAL_GetTick();
            }

            LCD_printString("GAME", 50, 70, 2, 6); 
            LCD_printString("OVER", 50, 140, 2, 6); 

            LCD_Refresh(&cfg0);

            if (HAL_GetTick() - lose_page_timer >= 2000) {
                lose_page_timer = 0;
                game_state = GAME_OVER; // Switch to game over page
            }
            break;

        case GAME_REWARD_PAGE:
            if (reward_page_timer == 0) {
                reward_page_timer = HAL_GetTick();
            }

            LCD_printString("REWARD", 30, 20, 10, 5); 
            LCD_printString("TIME!", 50, 70, 10, 5); 
            LCD_printString("You will receive", 20,130, 1, 2); 
            LCD_printString("LIFE +1 /", 70, 160, 4, 2); 
            LCD_printString("SHIELD MAX /", 50, 180, 11, 2); 
            LCD_printString("LIFE MAX", 70, 200, 3, 2); 

            LCD_Refresh(&cfg0);

            int r = rand () % 3;
            if (r == 0) {
                life++; 
            } else if (r == 1) {
                shield = SHIELD_MAX;
            } else {
                life = LIFE_MAX;
            }
            
            if (HAL_GetTick() - reward_page_timer >= 2000) {
                reward_page_timer = 0;
                status_level = 0;
                game_state = GAME_PLAYING;
            }
            break;

        case GAME_OVER:
            LCD_printString("THIS IS UN-", 20, 30, 10, 3); 
            LCD_printString("EGGCCEPTABLE!", 10, 70, 10, 3); 
            LCD_Draw_Sprite_Scaled(30, 110, 16, 16, (uint8_t *)EggLOSEPAGE, 5);
            LCD_printString("SCORE:", 140, 120, 1, 2); 
            sprintf(score_str, "%d", score);
            LCD_printString(score_str, 160, 150, 1, 3);
            LCD_printString("Press Joystick Btn", 10, 200, 1, 2);
            LCD_printString("To Restart", 60, 220, 1, 2);

            LCD_Refresh(&cfg0);

            if (gameover_melody_play == 0) {
                gameover_melody();          // Play game over melody on buzzer
                gameover_melody_play = 1;   // Change flag to 1
            }

            if (current_input.btn3_pressed) {
                current_input.btn3_pressed = 0; 
                life = LIFE_MAX;
                shield = SHIELD_MAX;
                score = 0;
                Character_Init(&game_character); 

                level_state = LEVEL1;
                status_level = 0;
                game_state = GAME_LEVEL_PAGE; 

                gameover_melody_play = 0;
            }
            break;
        }
}