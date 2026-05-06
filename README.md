# ELEC2645 Group 30 — STM32 Mini Game Console - Final Project

A collaborative embedded game console built on the STM32L476 microcontroller for the ELEC2645 Unit 4 Group Project. Three independently developed games are unified under a shared menu system and rendered on an ST7789V2 LCD display.

## Authors

| Game | Title | Author |
|------|-------|--------|
| Game 1 | **Maze Runner** | Anika Sandwar - mjsf7639|
| Game 2 | **Plants vs Zombies** | Fatima Hassan Khan - ldkv9439|
| Game 3 | **Chick Knight** | Aisya Afiqah Binti Mohd Fairoz Akasyah - gphg1759|
 
---
## Overview

This project provides:
- **Centralized Menu System**: Navigate between 3 independent games
- **Simple Game Loop**: INPUT → UPDATE → RENDER pattern
- **Shared Resources**: LCD display, joystick input, buzzer, PWM LED
- **Student Friendly**: Each student works in their own game folder
- **No Merge Conflicts**: Shared code stays in `shared/`, each game in its own folder

## Project Structure

```
MenuTest/
├── Core/              # STM32 auto-generated files
├── Drivers/           # STM32 HAL drivers
├── shared/            # Shared menu system & input handling
│   ├── Menu.h/c
│   └── InputHandler.h/c
├── game_1/            # Game 1 folder
│   └── Game_1.c
├── game_2/            # Game 2 folder
│   └── Game_2.c
├── game_3/            # Game 3 folder
│   └── Game_3.c
├── Joystick/          # Hardware drivers
├── PWM/
├── Buzzer/
└── CMakeLists.txt
```
## Games
 
### Game 1 — Maze Runner
Navigate through **5 progressively harder 13×13 mazes** before the clock runs out.
 
- **Controls:** Joystick to move · BT3 to return to menu
- **Objective:** Reach the exit tile on each level using as few moves as possible
- **Scoring:** Lowest total move count wins (high score tracked per session)
- **Difficulty scaling:** Each level reduces the time limit (60 → 35 → 28 → 22 → 18 s) and increases LED brightness
- **Fail condition:** Time expires → loud fail buzz → restart from Level 1
### Game 2 — Plants vs Zombies
A lane-defence game inspired by Plants vs Zombies, using a 6×6 grid with a checkerboard lawn, picket fence, house, and zombie spawn road.

- **Controls:** Joystick to move cursor · Joystick press to open plant menu/place plant· BT1 to return to menu
- **Objective:** Place different plants to stop waves of zombies from reaching the house
- **Plants:** Peashooter, Sunflower, Wallnut, Adv Peashooter, and Cherry Bomb — each with unique sun costs and abilities
- **Zombies:** Normal (200 HP) and Cone (500 HP) types spawning in increasing numbers across waves
- **Sun economy:** Start with 150 sun; earn more from falling tokens and Sunflowers placed on the grid
- **Engine:** Custom `PVZEngine` manages plants, zombies, projectiles, sun tokens, cursor, and wave logic in separate modules
- Runs at **~20 FPS** 
### Game 3 — Chick Knight
A top-down 2D arcade survival game where a chicken knight battles ghosts across multiple levels.
 
- **Controls:**
  - Joystick / BT3 — Move character & start/restart game
  - BT2 — Dash (temporary speed boost)
  - BT4 — Pause / Resume
- **Objective:** Collect coins and survive ghost attacks across escalating levels
- **Features:** Circle-based collision system, FSM-driven character animation, lava hazards, ghost bullets
- **Architecture:** Modular — Character, Coins, Ghost, Bullet, and Level are each independently managed objects
---
## Quick Start

See [README_STUDENTS.md](README_STUDENTS.md) for detailed student guide.

## Architecture

### Main Game Loop (main.c)

```c
while(1) {
    Input_Read();              // Read button and joystick
    
    switch(current_state) {    // UPDATE
        case MENU: Menu_Update(); break;
        case GAME_1: Game1_Update(); break;
        case GAME_2: Game2_Update(); break;
        case GAME_3: Game3_Update(); break;
    }
    
    switch(current_state) {    // RENDER
        case MENU: Menu_Render(); break;
        case GAME_1: Game1_Render(); break;
        case GAME_2: Game2_Render(); break;
        case GAME_3: Game3_Render(); break;
    }
}
```

### Each Game Implements Three Functions

```c
void GameX_Init(void);      // Called once when game is selected from menu
void GameX_Update(void);    // Called every frame (~30 FPS)
void GameX_Render(void);    // Called every frame (after Update)
```

## Controls

- **Joystick UP/DOWN**: Navigate menu
- **BT2 Button**: Available for custom game use
- **BT3 Button**: Select menu option or custom game use

## Hardware Features

- **STM32L476 Microcontroller**
- **ST7789V2 LCD Display** (240×320)
- **Joystick Input** with 8-directional output
- **PWM LED** for visual effects
- **Buzzer** for sound effects
- **Timers**: TIM6 (100Hz) and TIM7 (1Hz) available for game timing

See driver folders (Joystick/, PWM/, Buzzer/) for API documentation.
See [TIMER_USAGE_GUIDE.md](TIMER_USAGE_GUIDE.md) for timer examples.
