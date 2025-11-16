# PowerShell build script for Terraria Clone
# This script compiles the game without using CMake

Write-Host "Checking for g++ compiler..." -ForegroundColor Cyan

# Check if g++ is installed
if (!(Get-Command g++ -ErrorAction SilentlyContinue)) {
    Write-Host "Error: g++ compiler not found. Please install MinGW-w64 or MSYS2 first." -ForegroundColor Red
    Write-Host "Download from: https://www.msys2.org/" -ForegroundColor Yellow
    exit 1
}

# Create build directory if it doesn't exist
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Compile the source files
Write-Host "Compiling the game..." -ForegroundColor Cyan

g++ -std=c++17 `
    src/main.cpp `
    src/Game.cpp `
    src/World.cpp `
    src/Player.cpp `
    src/PerlinNoise.cpp `
    -o build/terraria_clone.exe `
    -Iinclude `
    -lsfml-graphics `
    -lsfml-window `
    -lsfml-system `
    -lBox2D

# Check if compilation was successful
if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful!" -ForegroundColor Green
    Write-Host "You can run the game with: .\build\terraria_clone.exe" -ForegroundColor Green
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
    Write-Host "Make sure you have SFML and Box2D libraries installed and in your compiler's library path." -ForegroundColor Yellow
    exit 1
}
