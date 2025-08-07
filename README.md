<img width="256" height="64" alt="Logo JOKER POKER PSP" align="right" src="https://github.com/user-attachments/assets/c1834f0c-3d2f-4a7d-98fa-e89788333080" />

# Joker Poker

Joker Poker is homebrew card game for PSP inspired by [Balatro](https://www.playbalatro.com).

Currently, development is focused on nailing down the basic systems.
Once these are solid, content will be expanded to include most of the jokers, boss blinds, tags, etc.

> [!NOTE]
> This project was written in plain C for the sake of writting it in C.

<p align="center">
   <img width="1072" height="684" alt="Screenshot of the game running on an emulator" src="https://github.com/user-attachments/assets/106bdeb9-f2c1-4512-923a-648d739b4a75" />
</p>

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
  - TRIANGLE - sell currently hovered item
- Game
  - SELECT - sort hand by rank/suit
  - SQUARE - play selected cards
  - TRIANGLE - discard selected cards
  - CIRCLE - deselect all cards
- Choose blind
  - CROSS - select current blind
  - TRIANGLE - skip current blind
- Booster Pack
  - CIRCLE - skip booster pack
