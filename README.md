# Terraria Clone

A 2D sandbox game inspired by Terraria, implemented in C++ with procedural world generation and player movement.

![Game Screenshot](https://p3-flow-imagex-sign.byteimg.com/tos-cn-i-a9rns2rl98/rc/pc/super_tool/63d9428154e94f63810b2afbd6b067f4~tplv-a9rns2rl98-image.image?rcl=20251116154141D2AE77DF93C85F2ED2D6&rk3s=8e244e95&rrcfp=f06b921b&x-expires=1765871263&x-signature=HIzECs08s8TKuodiDEEY6OhtSwY%3D)

## Features

- Procedural world generation using Perlin noise
- Player movement with collision detection
- Chunk-based world management
- Basic tile rendering system

## Prerequisites

Before you can build and run the game, you need to install the following dependencies:

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install -y cmake g++ libsfml-dev libbox2d-dev
```

### macOS

```bash
brew install cmake sfml box2d
```

### Windows

- Install [CMake](https://cmake.org/download/)
- Install [SFML](https://www.sfml-dev.org/download.php)
- Install [Box2D](https://box2d.org/download/)

## Building the Game

1. Clone the repository (if you haven't already):

```bash
git clone <repository-url>
cd terraria_clone
```

2. Create a build directory and navigate to it:

```bash
mkdir -p build
cd build
```

3. Run CMake to generate the build files:

```bash
cmake ..
```

4. Build the game:

```bash
make
```

On Windows, you can open the generated Visual Studio solution file and build it from there.

## Running the Game

After successfully building the game, you can run it from the build directory:

```bash
./terraria_clone
```

On Windows, you can run the executable from the build directory or from within Visual Studio.

## Controls

- **WASD**: Move the player
- **Space**: Jump
- **Escape**: Close the game

## Gameplay

- Explore the procedurally generated world
- Move around using the WASD keys
- Jump using the Space key
- The world is generated using Perlin noise, creating natural-looking terrain with grass, dirt, and caves

## Troubleshooting

### Missing Dependencies

If you encounter errors about missing libraries, make sure you have installed all the required dependencies as listed in the Prerequisites section.

### Compilation Errors

If you encounter compilation errors, try the following:

1. Make sure you're using a C++17 compatible compiler
2. Update your dependencies to the latest versions
3. Clean the build directory and try again:

```bash
cd build
rm -rf *
cmake ..
make
```

### Runtime Errors

If the game crashes or doesn't run properly:

1. Check that all required libraries are installed and available
2. Make sure you're running the game from the correct directory
3. Verify that the resources directory is present and contains the necessary files

## Extending the Game

Here are some ideas for extending the game:

1. Add block breaking and placing mechanics
2. Implement an inventory system
3. Add different types of blocks and resources
4. Create enemies and NPCs
5. Add crafting recipes
6. Implement a day/night cycle
7. Add biomes with different characteristics
8. Implement a lighting system

## License

This project is licensed under the MIT License - see the LICENSE file for details.
