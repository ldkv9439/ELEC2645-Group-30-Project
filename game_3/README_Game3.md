# Game 3: Chick Knight

A classic a 2D arcade survival game.

## Key Concepts:

- **Game objects** (Character, Coins, Ghost, Bullet, Flame): Has their own data and mechanics.
- **Collision system**: Check radius of the character with the objects.
- **Update/Render separation** : Game logic updates happen separately from drawing.

## Instructions:
- Collect all coins per level
- Avoid flame, bullets and ghosts
- Hits reduce shield first, and then life
- Shield and dash will recharge over time

## Controls:

**Joystick (BTN3)**
- Start game in the beginning
- Move character in all directions
- Restart game from Level 1 when pressed after win / lose game

**Push Button (BTN2)**
- Change character state to dashing mode (speed up movement)

**Push Button (BTN4):**
- Pause and resume game

## Architecture:
- **Character** : Main engine to control the character movement and states
- **Coins** : Handles coins spawning and drawing with position, rendering and collision 
- **Ghost and Bullet** : Handles ghosts and bullets spawning and drawing with position, movement, rendering and collision 
- **Flame**: Handles flame spawning and drawing with position, rendering and collision 
- **Level**: Handles amount of ghosts, coins and flame appearing per level
- **State**: Update and render each game state

## Project Structure

```
game_3/
├── Game_3.h/c         # Entry point, Game3_Run(), global variables
|                      # game_state, game_character, level_state
|
├── State.h/c          # Game state machine, update & render state  
|                      # Game_3_Update(), render_game(), draw_grid_background(), opening_page(), melodies and
|                        animation helpers
|
├── Character.h/c      # Player FSM, animation, movement and collision
|                      # Character_Init(), Character_Update(), Character_Draw()  
|
├── Ghost.h/c          # Ghost & bullet spawning, animation, movement and collision
|                      # Ghost_Add(), Ghost_Reset(), Ghost_Update(), Ghost_Bullet_Draw(), Bullet_Update()
|
├── Coins.h/c          # Coins spawning, collection and collision
|                      # Coins_Add(), Coins_Reset(), Coins_Update(), Coins_Draw(), Circles_Overlap()
|
├── Flame.h/c          # flame spawning, animation and collision
|                      # Flame_Add(), Flame_Reset(), Flame_Update(), Flame_Draw()
|
├── Level.h/c          # Level configuration
|                      # Level1(), Level2(), Level3(), BossLevel()
|
└── Sprites.h          # All sprites pixel data
                       # CharacterIDLE, CharacterWALKRIGHT_1/2, CharacterWALKLEFT, CharacterUPDOWN, CharacterDASHING 
                       # CharacterSTARTPAGE, ChickenWINPAGE, EggLOSEPAGE, GhostIDLE1/2, FlameIDLE1/2

```

### Main Game Loop (main.c)

```c
while(1) {
    Input_Read();              // Read button and joystick
    
    Game3_Update(&joystick_data);  // Update game
}
```

### Game States

```c
Game3_Update() {
    switch(game_state) {    
        case GAME_START_PAGE:       break;
        case GAME_INSTRUCTION_PAGE: break;
        case GAME_LEVEL_PAGE:       break;
        case GAME_PLAYING:          break;
        case GAME_WIN:              break;
        case GAME_PAUSE:            break;
        case GAME_LOSE_PAGE:        break;
        case GAME_REWARD_PAGE:      break;
        case GAME_OVER:             break;
    }
}
```

## Hardware Features

- **STM32L476 Microcontroller**
- **ST7789V2 LCD Display** (240×320)
- **Joystick Input** with 8-directional output
- **PWM LED** for visual effects (Red LED: collision with ghost/dashing mode, Yellow LED: coins collection)
- **Buzzer** for sound effects
- **Timers**: TIM6 (100Hz) and TIM7 (1Hz) available for game timing