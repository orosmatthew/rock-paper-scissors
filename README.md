# Rock Paper Scissors Simulation

![Game Image](https://raw.githubusercontent.com/orosmatthew/rock-paper-scissors/master/image/game.gif)

## Description

This is a simple simulation of rock paper scissors inspired by a post I saw online. It uses C++20 along with the
fantastic Raylib library.

## Web Demo

The web demo can be found on my website
at [https://www.pixeled.site/projects/rock-paper-scissors/](https://www.pixeled.site/projects/rock-paper-scissors/)

## Build Instructions

### Desktop

```bash
git clone https://github.com/orosmatthew/rock-paper-scissors
cd rock-paper-scissors
cmake -S . -B build
cmake --build build
# Then copy res/ folder to the same folder as the executable
```

### Web

> NOTE: requires Emscripten (emsdk)

```bash
git clone https://github.com/orosmatthew/rock-paper-scissors
cd rock-paper-scissors
cmake -S . -B build -DPLATFORM=Web --toolchain <fullpath_to_emsdk>/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
# Then copy res/ folder to build/ folder
cmake --build build
# Then copy /dist/index.html to build/ folder
```

Run by hosting the `index.html`, `rock_paper_scissors.data` file, `rock_paper_scissors.js` file,
and `rock_paper_scissors.wasm` file.
