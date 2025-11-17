#ifndef CONSTANTS_H
#define CONSTANTS_H

// Window constants
constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr const char* WINDOW_TITLE = "Terraria Clone";

// Game constants
constexpr float GAME_SPEED = 60.0f;
constexpr float TIME_STEP = 1.0f / GAME_SPEED;

// Tile constants
constexpr int TILE_SIZE = 16;
constexpr int TILE_TYPES = 11;

// Chunk constants
constexpr int CHUNK_WIDTH = 32;
constexpr int CHUNK_HEIGHT = 32;
constexpr int CHUNKS_RENDERED = 10;

// Player constants (per-second units)
constexpr float PLAYER_MAX_SPEED = 180.0f;
constexpr float PLAYER_ACCELERATION = 900.0f;
constexpr float PLAYER_DECELERATION = 900.0f;
constexpr float PLAYER_AIR_DECELERATION = 400.0f;
constexpr float PLAYER_JUMP_VELOCITY = 320.0f;
constexpr float PLAYER_GRAVITY = 900.0f;
constexpr int PLAYER_WIDTH = 16;
constexpr int PLAYER_HEIGHT = 32;

// World constants
constexpr int WORLD_WIDTH = 100;
constexpr int WORLD_HEIGHT = 500;
constexpr float NOISE_SCALE = 0.05f;
constexpr int OCTAVES = 4;
constexpr float PERSISTENCE = 0.5f;
constexpr float LACUNARITY = 2.0f;

#endif // CONSTANTS_H
