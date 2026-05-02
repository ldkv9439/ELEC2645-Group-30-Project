#ifndef GAME_2_H
#define GAME_2_H

#include "Menu.h"
#include "PVZEngine.h"

/**
 * @brief Game 2 - Plants vs Zombies Implementation
 * 
 * This module implements a full Plants vs Zombies game that runs
 * as a selectable game from the main menu system.
 * 
 * The game uses the Game_2_Run() function which is called by the menu
 * system when Game 2 is selected. The function returns control to the
 * menu when the game exits.
 * 
 * Key features:
 * - Full PvZ game engine integration
 * - Joystick/input handling
 * - LCD rendering with scrolling background
 * - Sound effects via buzzer
 * - Proper frame timing (~20 FPS)
 * - Menu integration for seamless switching
 * 
 * @author Game 2 Team
 * @date 2026
 */

/**
 * @brief Run Game 2 - Plants vs Zombies
 * 
 * Main entry point for the Plants vs Zombies game.
 * This function initializes the game engine, runs the main game loop,
 * and returns control to the menu system when the game exits.
 * 
 * The game loop:
 * 1. Reads joystick input
 * 2. Updates game state
 * 3. Renders to LCD with proper frame timing
 * 4. Checks for exit conditions (menu button or game over)
 * 
 * Frame rate: ~20 FPS (50ms per frame)
 * Screen: 240x240 LCD with HUD at top
 * 
 * @return MenuState - Where the menu system should go next
 *         MENU_STATE_HOME - return to main menu (typical)
 *         Other states - could be extended for level select, etc.
 * 
 * @note This function must be called from the menu system's game selection
 * @note The function will block until the game exits
 * @note All hardware (LCD, buzzer, joystick, etc.) must be initialized before calling
 */
MenuState Game2_Run(void);

#endif // GAME_2_H
