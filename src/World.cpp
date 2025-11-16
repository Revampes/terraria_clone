#include "World.h"
#include <algorithm>
#include <cmath>

World::World() {
    // Initialize Perlin noise with a seed
    perlin = PerlinNoise(12345);
    
    // Load textures
    if (!dirtTexture.loadFromFile("resources/Tiles_0.png")) {
        // Handle error - texture not loaded
    }
    if (!stoneTexture.loadFromFile("resources/Tiles_1.png")) {
        // Handle error - texture not loaded
    }
    
    // Generate initial world
    generateWorld();
}

World::~World() {}

void World::generateWorld() {
    // Generate chunks around the initial player position
    for (int y = -CHUNKS_RENDERED / 2; y < CHUNKS_RENDERED / 2; ++y) {
        for (int x = -CHUNKS_RENDERED / 2; x < CHUNKS_RENDERED / 2; ++x) {
            generateChunk(x, y);
        }
    }
}

void World::generateChunk(int chunkX, int chunkY) {
    Chunk chunk;
    chunk.x = chunkX;
    chunk.y = chunkY;
    chunk.generated = true;
    
    // Initialize tiles
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            int worldX = chunkX * CHUNK_WIDTH + x;
            int worldY = chunkY * CHUNK_HEIGHT + y;
            
            Tile tile;
            tile.type = 0;
            tile.solid = false;
            tile.visible = false;
            
            // Generate terrain using Perlin noise
            float noiseValue = perlin.octaveNoise(worldX * NOISE_SCALE, worldY * NOISE_SCALE, OCTAVES, PERSISTENCE, LACUNARITY);
            
            // Determine tile type based on noise value and height
            if (worldY > 30 + noiseValue * 10) {
                // Deep underground - stone and ores
                if (worldY > 35 + noiseValue * 10) {
                    tile.type = 3; // Stone
                    tile.solid = true;
                    tile.visible = true;
                    
                    // Generate ores (made less rare)
                    float oreNoise = perlin.octaveNoise(worldX * 0.1f, worldY * 0.1f, 2, 0.5f, 2.0f);
                    
                    // Coal ore (common)
                    if (oreNoise > 0.55f && oreNoise < 0.65f) {
                        tile.type = 4; // Coal
                    }
                    // Iron ore (uncommon)
                    else if (oreNoise > 0.35f && oreNoise < 0.42f && worldY > 40) {
                        tile.type = 5; // Iron
                    }
                    // Gold ore (less rare)
                    else if (oreNoise > 0.7f && oreNoise < 0.76f && worldY > 50) {
                        tile.type = 6; // Gold
                    }
                    // Diamond ore (rare but more common than before)
                    else if (oreNoise > 0.15f && oreNoise < 0.18f && worldY > 60) {
                        tile.type = 7; // Diamond
                    }
                } else {
                    tile.type = 1; // Dirt
                    tile.solid = true;
                    tile.visible = true;
                }
            } else if (worldY > 25 + noiseValue * 10) {
                tile.type = 2; // Grass
                tile.solid = true;
                tile.visible = true;
            } else {
                tile.type = 0; // Air
                tile.solid = false;
                tile.visible = false;
            }
            
            // Add some caves
            float caveNoise = perlin.octaveNoise(worldX * NOISE_SCALE * 2, worldY * NOISE_SCALE * 2, 3, 0.5, 2.0);
            if (worldY > 10 && worldY < 450 && caveNoise > 0.7 && tile.solid) {
                tile.type = 0; // Air
                tile.solid = false;
                tile.visible = false;
            }
            
            chunk.tiles[x][y] = tile;
        }
    }
    
    // Update chunk vertices for rendering
    updateChunkVertices(chunk);
    
    // Add chunk to the list
    chunks.push_back(chunk);
}

void World::draw(sf::RenderWindow& window, const sf::Vector2f& playerPosition) {
    // Calculate which chunks to render based on player position
    int playerChunkX = static_cast<int>(std::floor(playerPosition.x / (CHUNK_WIDTH * TILE_SIZE)));
    int playerChunkY = static_cast<int>(std::floor(playerPosition.y / (CHUNK_HEIGHT * TILE_SIZE)));
    
    // Render chunks around the player
    for (const Chunk& chunk : chunks) {
        if (abs(chunk.x - playerChunkX) <= CHUNKS_RENDERED / 2 && 
            abs(chunk.y - playerChunkY) <= CHUNKS_RENDERED / 2) {
            
            // Calculate chunk position in pixels
            sf::Vector2f chunkPosition(chunk.x * CHUNK_WIDTH * TILE_SIZE, 
                                      chunk.y * CHUNK_HEIGHT * TILE_SIZE);
            
            // Separate vertices by tile type for proper texture rendering
            sf::VertexArray dirtVertices(sf::Quads);
            sf::VertexArray stoneVertices(sf::Quads);
            sf::VertexArray otherVertices(sf::Quads);
            
            // Go through each tile and add to appropriate vertex array
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int x = 0; x < CHUNK_WIDTH; ++x) {
                    Tile tile = chunk.tiles[x][y];
                    
                    if (tile.visible) {
                        sf::Vertex quad[4];
                        quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE) + chunkPosition;
                        quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE) + chunkPosition;
                        quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE) + chunkPosition;
                        quad[3].position = sf::Vector2f(x * TILE_SIZE, (y + 1) * TILE_SIZE) + chunkPosition;
                        
                        quad[0].texCoords = sf::Vector2f(0, 0);
                        quad[1].texCoords = sf::Vector2f(16, 0);
                        quad[2].texCoords = sf::Vector2f(16, 16);
                        quad[3].texCoords = sf::Vector2f(0, 16);
                        
                        quad[0].color = sf::Color::White;
                        quad[1].color = sf::Color::White;
                        quad[2].color = sf::Color::White;
                        quad[3].color = sf::Color::White;
                        
                        if (tile.type == 1) {
                            // Dirt - use dirt texture
                            dirtVertices.append(quad[0]);
                            dirtVertices.append(quad[1]);
                            dirtVertices.append(quad[2]);
                            dirtVertices.append(quad[3]);
                        } else if (tile.type == 3) {
                            // Stone - use stone texture
                            stoneVertices.append(quad[0]);
                            stoneVertices.append(quad[1]);
                            stoneVertices.append(quad[2]);
                            stoneVertices.append(quad[3]);
                        } else {
                            // Other tiles - use color
                            if (tile.type == 2) {
                                quad[0].color = sf::Color(34, 139, 34);
                                quad[1].color = sf::Color(34, 139, 34);
                                quad[2].color = sf::Color(34, 139, 34);
                                quad[3].color = sf::Color(34, 139, 34);
                            } else if (tile.type == 4) {
                                quad[0].color = sf::Color(50, 50, 50);
                                quad[1].color = sf::Color(50, 50, 50);
                                quad[2].color = sf::Color(50, 50, 50);
                                quad[3].color = sf::Color(50, 50, 50);
                            } else if (tile.type == 5) {
                                quad[0].color = sf::Color(205, 127, 50);
                                quad[1].color = sf::Color(205, 127, 50);
                                quad[2].color = sf::Color(205, 127, 50);
                                quad[3].color = sf::Color(205, 127, 50);
                            } else if (tile.type == 6) {
                                quad[0].color = sf::Color(255, 215, 0);
                                quad[1].color = sf::Color(255, 215, 0);
                                quad[2].color = sf::Color(255, 215, 0);
                                quad[3].color = sf::Color(255, 215, 0);
                            } else if (tile.type == 7) {
                                quad[0].color = sf::Color(0, 191, 255);
                                quad[1].color = sf::Color(0, 191, 255);
                                quad[2].color = sf::Color(0, 191, 255);
                                quad[3].color = sf::Color(0, 191, 255);
                            }
                            otherVertices.append(quad[0]);
                            otherVertices.append(quad[1]);
                            otherVertices.append(quad[2]);
                            otherVertices.append(quad[3]);
                        }
                    }
                }
            }
            
            // Draw each batch with appropriate texture
            if (dirtVertices.getVertexCount() > 0) {
                window.draw(dirtVertices, &dirtTexture);
            }
            if (stoneVertices.getVertexCount() > 0) {
                window.draw(stoneVertices, &stoneTexture);
            }
            if (otherVertices.getVertexCount() > 0) {
                window.draw(otherVertices);
            }
        }
    }
}

Tile World::getTile(int x, int y) const {
    // Find the chunk containing the tile
    int chunkX = x / CHUNK_WIDTH;
    int chunkY = y / CHUNK_HEIGHT;
    
    // Handle negative coordinates properly for modulo
    int tileX = x % CHUNK_WIDTH;
    int tileY = y % CHUNK_HEIGHT;
    if (tileX < 0) { tileX += CHUNK_WIDTH; chunkX--; }
    if (tileY < 0) { tileY += CHUNK_HEIGHT; chunkY--; }
    
    for (const Chunk& chunk : chunks) {
        if (chunk.x == chunkX && chunk.y == chunkY) {
            return chunk.tiles[tileX][tileY];
        }
    }
    
    // Return air tile if chunk not found
    Tile airTile;
    airTile.type = 0;
    airTile.solid = false;
    airTile.visible = false;
    return airTile;
}

void World::setTile(int x, int y, Tile tile) {
    // Find the chunk containing the tile
    int chunkX = x / CHUNK_WIDTH;
    int chunkY = y / CHUNK_HEIGHT;
    
    // Handle negative coordinates properly for modulo
    int tileX = x % CHUNK_WIDTH;
    int tileY = y % CHUNK_HEIGHT;
    if (tileX < 0) { tileX += CHUNK_WIDTH; chunkX--; }
    if (tileY < 0) { tileY += CHUNK_HEIGHT; chunkY--; }
    
    for (Chunk& chunk : chunks) {
        if (chunk.x == chunkX && chunk.y == chunkY) {
            chunk.tiles[tileX][tileY] = tile;
            updateChunkVertices(chunk);
            return;
        }
    }
}

Chunk* World::getChunk(int x, int y) {
    for (Chunk& chunk : chunks) {
        if (chunk.x == x && chunk.y == y) {
            return &chunk;
        }
    }
    return nullptr;
}

bool World::chunkExists(int x, int y) const {
    for (const Chunk& chunk : chunks) {
        if (chunk.x == x && chunk.y == y) {
            return true;
        }
    }
    return false;
}

void World::updateChunks(int playerChunkX, int playerChunkY) {
    // Generate chunks around player
    int renderDistance = CHUNKS_RENDERED / 2;
    
    for (int y = playerChunkY - renderDistance; y <= playerChunkY + renderDistance; ++y) {
        for (int x = playerChunkX - renderDistance; x <= playerChunkX + renderDistance; ++x) {
            if (!chunkExists(x, y)) {
                generateChunk(x, y);
            }
        }
    }
    
    // Unload distant chunks to save memory
    unloadDistantChunks(playerChunkX, playerChunkY);
}

void World::unloadDistantChunks(int playerChunkX, int playerChunkY) {
    int unloadDistance = CHUNKS_RENDERED + 2; // Keep some extra chunks loaded
    
    chunks.erase(
        std::remove_if(chunks.begin(), chunks.end(),
            [playerChunkX, playerChunkY, unloadDistance](const Chunk& chunk) {
                int dx = abs(chunk.x - playerChunkX);
                int dy = abs(chunk.y - playerChunkY);
                return dx > unloadDistance || dy > unloadDistance;
            }),
        chunks.end()
    );
}

void World::updateChunkVertices(Chunk& chunk) {
    chunk.vertices.setPrimitiveType(sf::Quads);
    chunk.vertices.resize(CHUNK_WIDTH * CHUNK_HEIGHT * 4);
    
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            Tile tile = chunk.tiles[x][y];
            
            // Get a pointer to the current tile's quad
            sf::Vertex* quad = &chunk.vertices[(x + y * CHUNK_WIDTH) * 4];
            
            if (tile.visible) {
                // Set quad position
                quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
                quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE);
                quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
                quad[3].position = sf::Vector2f(x * TILE_SIZE, (y + 1) * TILE_SIZE);
                
                // Set texture coordinates to use full texture (16x16 pixels)
                quad[0].texCoords = sf::Vector2f(0, 0);
                quad[1].texCoords = sf::Vector2f(16, 0);
                quad[2].texCoords = sf::Vector2f(16, 16);
                quad[3].texCoords = sf::Vector2f(0, 16);
                
                // Set color to white so texture shows properly
                quad[0].color = sf::Color::White;
                quad[1].color = sf::Color::White;
                quad[2].color = sf::Color::White;
                quad[3].color = sf::Color::White;
                
                // For tile types without textures yet, use colored fallback
                if (tile.type == 2) {
                    // Grass tile - use green color
                    quad[0].color = sf::Color(34, 139, 34);
                    quad[1].color = sf::Color(34, 139, 34);
                    quad[2].color = sf::Color(34, 139, 34);
                    quad[3].color = sf::Color(34, 139, 34);
                } else if (tile.type == 4) {
                    // Coal ore
                    quad[0].color = sf::Color(50, 50, 50);
                    quad[1].color = sf::Color(50, 50, 50);
                    quad[2].color = sf::Color(50, 50, 50);
                    quad[3].color = sf::Color(50, 50, 50);
                } else if (tile.type == 5) {
                    // Iron ore
                    quad[0].color = sf::Color(205, 127, 50);
                    quad[1].color = sf::Color(205, 127, 50);
                    quad[2].color = sf::Color(205, 127, 50);
                    quad[3].color = sf::Color(205, 127, 50);
                } else if (tile.type == 6) {
                    // Gold ore
                    quad[0].color = sf::Color(255, 215, 0);
                    quad[1].color = sf::Color(255, 215, 0);
                    quad[2].color = sf::Color(255, 215, 0);
                    quad[3].color = sf::Color(255, 215, 0);
                } else if (tile.type == 7) {
                    // Diamond ore
                    quad[0].color = sf::Color(0, 191, 255);
                    quad[1].color = sf::Color(0, 191, 255);
                    quad[2].color = sf::Color(0, 191, 255);
                    quad[3].color = sf::Color(0, 191, 255);
                }
            } else {
                // For invisible tiles, set vertices to transparent and collapse to a single point
                quad[0].position = sf::Vector2f(0, 0);
                quad[1].position = sf::Vector2f(0, 0);
                quad[2].position = sf::Vector2f(0, 0);
                quad[3].position = sf::Vector2f(0, 0);
                
                quad[0].color = sf::Color::Transparent;
                quad[1].color = sf::Color::Transparent;
                quad[2].color = sf::Color::Transparent;
                quad[3].color = sf::Color::Transparent;
            }
        }
    }
}
