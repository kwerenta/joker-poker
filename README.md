# Joker Poker

Joker Poker is homebrew card game for PSP inspired by [Balatro](https://www.playbalatro.com).

Currently, development is focused on nailing down the basic systems.
Once these are solid, content will be expanded to include most of the jokers, boss blinds, tags, etc.

> [!NOTE]
> This project was written in plain C for the sake of writting it in C.

## Installation

### Installation from zip file

1. Go to [releases page](https://github.com/kwerenta/joker-poker/releases)
1. Download latest `joker-poker.zip` file
1. Extract downloaded zip file on the Memory Stick in `/PSP/GAME/`
1. Game is ready to launch

### Installation from source

1. Compile the game (see [Compiling](#compiling))
1. On the Memory Stick of the physical PSP or emulator create a new folder in `/PSP/GAME/`
1. Copy both `EBOOT.PBP` file from `build` directory and `res` folder to newly created folder
1. Game is ready to launch

## Compiling

### Prerequisites

- [PSP SDK](https://pspdev.github.io)

### Steps

1. Clone the repository

2. Then generate Makefile inside `build` directory, to do this just enter at the command line:

   ```sh
   psp-cmake -B build
   ```

   - In order to make game work on unmodded PSP (with official firmware), add following flags to above command:

     ```sh
     -DBUILD_PRX=1 -DENC_PRX=1
     ```

   - In order to make game work in debug mode (Game will generate log file), add following flag to above command:
     ```sh
     -DCMAKE_BUILD_TYPE=Debug
     ```

3. Now, you can build game with:
   ```sh
   cmake --build build
   ```

After the first build, running last step is enough to get an `EBOOT.PBP` file which is main game binary.

## Controls

There are currently no in-game control hints.

- General
  - UP/DOWN/LEFT/RIGHT - move between different sections
  - LEFT/RIGHT - change currently hovered item
  - CROSS - select/buy/use currently hovered item
  - LTRIGGER/RTRIGGER - move hovered item to the left/right
  - START - open/close overlay menu
- Game
  - SELECT - sort hand by rank/suit
  - SQUARE - play selected cards
  - TRIANGLE - discard selected cards
  - CIRCLE - deselect all cards
- BOOSTER PACK
  - CIRCLE - skip booster pack
