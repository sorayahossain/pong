# Pong

A simple Pong game, built as a learning project.

The `main` branch holds the base code. It will be evolved, step by step, into a
full game with a 4-paddle survival multiplayer mode: one human paddle facing
three AI paddles, where the last paddle standing wins.

Check out [Pull Request #1](https://github.com/sorayahossain/pong/pull/1) for a
step-by-step solution: each commit message explains in detail what that
change does and why.

## Dependencies

- **Windows**: Visual Studio. [raylib](https://www.raylib.com/) is already
  vendored as prebuilt binaries, so no extra setup is needed.
- **macOS**: CMake and raylib.
- **Ubuntu**: CMake and raylib.

## Installing dependencies

### Ubuntu

```sh
sudo apt update
sudo apt install build-essential cmake git \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev \
    libxinerama-dev libwayland-dev libxkbcommon-dev
```

CMake will fetch and build raylib automatically if it's not already installed
on your system.

### macOS

```sh
xcode-select --install
brew install cmake
```

CMake will fetch and build raylib automatically if it's not already installed
on your system.

## Build and run

From the repository root:

```sh
cmake -S . -B build
cmake --build build
./build/Pong
```
