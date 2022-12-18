# Rock Paper Scissors Simulation

![Game Image](https://raw.githubusercontent.com/orosmatthew/rock-paper-scissors/master/image/game.gif)

## Description

This is a simple simulation of rock paper scissors inspired by a post I saw online. It uses C++20 along with the fantastic Raylib library.

## Build Instructions

```bash
git clone https://github.com/orosmatthew/rock-paper-scissors
cd rock-paper-scissors
cmake -S . -B build
cmake --build build
```

The executable will be in the `build/` folder but will be different depending on the generator CMake uses.

> NOTE: You must copy the `res/` resources directory into the same location as the executable, otherwise the game will not be able to load the assets!

