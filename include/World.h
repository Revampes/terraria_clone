#ifndef WORLD_H
#define WORLD_H

#include <random>
#include <vector>
#include <SFML/Graphics.hpp>
#include "PerlinNoise.h"
#include "Constants.h"

struct Tile {
    int type = 0;
    bool solid = false;
    bool visible = false;
    bool fogEnabled = true;
};

struct Chunk {
    Tile tiles[CHUNK_WIDTH][CHUNK_HEIGHT];
    int x, y;
    bool generated;
    sf::VertexArray vertices;
};

class World {
public:
    World();
    ~World();
    
    void generateWorld();
    void generateChunk(int chunkX, int chunkY);
    void generateTrees(int chunkX, int chunkY);
    void draw(sf::RenderWindow& window, const sf::Vector2f& playerPosition);
    void updateChunks(int playerChunkX, int playerChunkY);
    
    Tile getTile(int x, int y) const;
    void setTile(int x, int y, Tile tile);
    
private:
    std::vector<Chunk> chunks;
    PerlinNoise perlin;
    sf::Texture dirtTexture;
    sf::Texture stoneTexture;
    std::mt19937 rng;
    std::uniform_int_distribution<int> treeHeightDistribution;
    
    Chunk* getChunk(int x, int y);
    void updateChunkVertices(Chunk& chunk);
    bool chunkExists(int x, int y) const;
    void unloadDistantChunks(int playerChunkX, int playerChunkY);
    static void computeChunkCoords(int worldX, int worldY, int& chunkX, int& chunkY, int& tileX, int& tileY);
    bool locateTileMutable(int worldX, int worldY, Chunk*& chunkOut, int& tileX, int& tileY);
    void recalcChunkVisibility(Chunk& chunk);
    bool isAirTile(int worldX, int worldY) const;
    bool shouldTileBeVisible(int worldX, int worldY, const Tile& tile) const;
    void refreshVisibilityAround(int worldX, int worldY);
    void refreshNeighborChunks(const Chunk& chunk);
};

#endif // WORLD_H
