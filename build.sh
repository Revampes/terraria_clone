#!/bin/bash

# Simple build script for Terraria Clone
# This script compiles the game without using CMake

# Check if g++ is installed
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ compiler not found. Please install g++ first."
    exit 1
fi

# Check if SFML is installed
if ! pkg-config --exists sfml-graphics sfml-window sfml-system; then
    echo "Error: SFML library not found. Please install SFML first."
    echo "On Ubuntu/Debian: sudo apt-get install libsfml-dev"
    echo "On macOS: brew install sfml"
    exit 1
fi

# Create build directory if it doesn't exist
mkdir -p build

# Compile the source files
echo "Compiling the game..."
g++ -std=c++17 \
    src/main.cpp \
    src/Game.cpp \
    src/World.cpp \
    src/Player.cpp \
    src/PerlinNoise.cpp \
    src/PlayerRenderer.cpp \
    src/Items.cpp \
    src/Inventory.cpp \
    -o build/terraria_clone \
    -Iinclude \
    $(pkg-config --cflags --libs sfml-graphics sfml-window sfml-system)

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo "You can run the game with: ./build/terraria_clone"
else
    echo "Compilation failed!"
    exit 1
fi
