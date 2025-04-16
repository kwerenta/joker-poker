# Joker Poker

Joker Poker is homebrew card game for PSP inspired by [Balatro](https://www.playbalatro.com).

> [!NOTE]
> This project was written in plain C for the sake of writting it in C.

## Installation

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

```sh
git clone https://github.com/kwerenta/joker-poker
cd joker-poker
```

2. In order to generate Makefile inside `build` directory, just enter at the command line:

```sh
psp-cmake -B build
```

    - In order to make game work on unmodded PSP (with official firmware), add following flags to above command:

```sh
-DBUILD_PRX=1 -DENC_PRX=1
```

    - In order to make game work in debug mode (Game will generate log file), add following flags to above command:

```sh
-DCMAKE_BUILD_TYPE=Debug
```

3. Now, you can build game with:

```sh
cmake --build build
```

or

```sh
cd build
make
```

After the first build, running last step is enough to get an `EBOOT.PBP` file which is main game binary.
