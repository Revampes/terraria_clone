#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <SFML/Graphics.hpp>
#include "PerlinNoise.h"
#include "Constants.h"

struct Tile {
    int type;
    bool solid;
    bool visible;
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
    
    Chunk* getChunk(int x, int y);
    void updateChunkVertices(Chunk& chunk);
    bool chunkExists(int x, int y) const;
    void unloadDistantChunks(int playerChunkX, int playerChunkY);
};

#endif // WORLD_H
