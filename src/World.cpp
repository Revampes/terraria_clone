#include "World.h"
#include <algorithm>
#include <cmath>

World::World()
        : rng(std::random_device{}()),
            treeHeightDistribution(4, 10) {
    // Initialize Perlin noise with a seed
    perlin = PerlinNoise(12345);
    
    // Load textures
    if (!dirtTexture.loadFromFile("resources/Tiles_0.png")) {
        // Handle error - texture not loaded
    } else {
        dirtTexture.setSmooth(false); // Disable smoothing for pixel-perfect rendering
        dirtTexture.setRepeated(true);
    }
    if (!stoneTexture.loadFromFile("resources/Tiles_1.png")) {
        // Handle error - texture not loaded
    } else {
        stoneTexture.setSmooth(false); // Disable smoothing for pixel-perfect rendering
        stoneTexture.setRepeated(true);
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
    
    // Generate trees after all initial terrain is created
    for (int y = -CHUNKS_RENDERED / 2; y < CHUNKS_RENDERED / 2; ++y) {
        for (int x = -CHUNKS_RENDERED / 2; x < CHUNKS_RENDERED / 2; ++x) {
            generateTrees(x, y);
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
            tile.fogEnabled = true;
            
            // Generate terrain using Perlin noise
            float noiseValue = perlin.octaveNoise(worldX * NOISE_SCALE, worldY * NOISE_SCALE, OCTAVES, PERSISTENCE, LACUNARITY);
            
            // Calculate surface height
            int surfaceHeight = static_cast<int>(30 + noiseValue * 10);
            
            // Determine tile type based on noise value and height
            if (worldY > surfaceHeight) {
                // Underground - dirt then stone
                if (worldY > surfaceHeight + 5) {
                    // Deep underground - stone and ores
                    tile.type = 3; // Stone
                    tile.solid = true;
                    
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
                }
            } else if (worldY == surfaceHeight) {
                // Surface layer - grass (only one layer)
                tile.type = 2; // Grass
                tile.solid = true;
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
    
    // Store chunk, then compute visibility and vertices
    chunks.push_back(chunk);
    Chunk& storedChunk = chunks.back();
    recalcChunkVisibility(storedChunk);
    updateChunkVertices(storedChunk);
    refreshNeighborChunks(storedChunk);
}

void World::generateTrees(int chunkX, int chunkY) {
    // Generate trees after terrain is created
    // Only generate trees for chunks near surface (y <= 2)
    if (chunkY > 2) return;
    
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        int worldX = chunkX * CHUNK_WIDTH + x;
        
        // Use simple spacing - tree every 8-12 blocks
        if (worldX % 10 == 0) {
            // Find the surface by scanning downward from top of this chunk
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                int worldY = chunkY * CHUNK_HEIGHT + y;
                
                // Look for grass blocks with air above
                Tile currentTile = getTile(worldX, worldY);
                Tile aboveTile = getTile(worldX, worldY - 1);
                
                if (currentTile.type == 2 && aboveTile.type == 0) {
                    // Found surface! Place a tree
                    int treeHeight = treeHeightDistribution(rng);
                    
                    // Place trunk
                    for (int h = 1; h <= treeHeight; ++h) {
                        Tile woodTile;
                        woodTile.type = 8; // Wood
                        woodTile.solid = false; // Non-solid so player can walk through
                        woodTile.visible = false;
                        woodTile.fogEnabled = false;
                        setTile(worldX, worldY - h, woodTile);
                    }
                    
                    // Generate leaves in a simple pattern
                    int leafY = worldY - treeHeight;
                    for (int ly = -2; ly <= 1; ++ly) {
                        for (int lx = -2; lx <= 2; ++lx) {
                            // Skip corners
                            if (std::abs(lx) == 2 && std::abs(ly) == 2) continue;
                            // Skip trunk
                            if (lx == 0 && ly > -2) continue;
                            
                            Tile existingTile = getTile(worldX + lx, leafY + ly);
                            if (existingTile.type == 0) {
                                Tile leafTile;
                                leafTile.type = 9; // Leaves
                                leafTile.solid = false; // Non-solid so player can walk through
                                leafTile.visible = false;
                                leafTile.fogEnabled = false;
                                setTile(worldX + lx, leafY + ly, leafTile);
                            }
                        }
                    }
                    
                    break; // Only one tree per column
                }
            }
        }
    }
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
            sf::VertexArray hiddenVertices(sf::Quads);
            
            // Go through each tile and add to appropriate vertex array
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int x = 0; x < CHUNK_WIDTH; ++x) {
                    int worldX = chunk.x * CHUNK_WIDTH + x;
                    int worldY = chunk.y * CHUNK_HEIGHT + y;
                    Tile tile = chunk.tiles[x][y];

                    if (tile.type == 0) {
                        continue;
                    }

                    sf::Vertex quad[4];
                    quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE) + chunkPosition;
                    quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE) + chunkPosition;
                    quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE) + chunkPosition;
                    quad[3].position = sf::Vector2f(x * TILE_SIZE, (y + 1) * TILE_SIZE) + chunkPosition;

                    if (!tile.visible && tile.fogEnabled) {
                        sf::Color fogShade(0, 0, 0, 235);
                        quad[0].color = fogShade;
                        quad[1].color = fogShade;
                        quad[2].color = fogShade;
                        quad[3].color = fogShade;
                        hiddenVertices.append(quad[0]);
                        hiddenVertices.append(quad[1]);
                        hiddenVertices.append(quad[2]);
                        hiddenVertices.append(quad[3]);
                        continue;
                    } else if (!tile.visible) {
                        continue;
                    }

                    // Use first tile from atlas with small inset to avoid transparent borders
                    quad[0].texCoords = sf::Vector2f(0.5f, 0.5f);
                    quad[1].texCoords = sf::Vector2f(15.5f, 0.5f);
                    quad[2].texCoords = sf::Vector2f(15.5f, 15.5f);
                    quad[3].texCoords = sf::Vector2f(0.5f, 15.5f);

                    quad[0].color = sf::Color::White;
                    quad[1].color = sf::Color::White;
                    quad[2].color = sf::Color::White;
                    quad[3].color = sf::Color::White;

                    if (tile.type == 1) {
                        dirtVertices.append(quad[0]);
                        dirtVertices.append(quad[1]);
                        dirtVertices.append(quad[2]);
                        dirtVertices.append(quad[3]);
                    } else if (tile.type == 3) {
                        stoneVertices.append(quad[0]);
                        stoneVertices.append(quad[1]);
                        stoneVertices.append(quad[2]);
                        stoneVertices.append(quad[3]);
                    } else {
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
                        } else if (tile.type == 8) {
                            quad[0].color = sf::Color(139, 69, 19);
                            quad[1].color = sf::Color(139, 69, 19);
                            quad[2].color = sf::Color(139, 69, 19);
                            quad[3].color = sf::Color(139, 69, 19);
                        } else if (tile.type == 9) {
                            quad[0].color = sf::Color(34, 139, 34, 180);
                            quad[1].color = sf::Color(34, 139, 34, 180);
                            quad[2].color = sf::Color(34, 139, 34, 180);
                            quad[3].color = sf::Color(34, 139, 34, 180);
                        } else if (tile.type == 10) {
                            quad[0].color = sf::Color(205, 170, 125);
                            quad[1].color = sf::Color(205, 170, 125);
                            quad[2].color = sf::Color(205, 170, 125);
                            quad[3].color = sf::Color(205, 170, 125);
                        }
                        otherVertices.append(quad[0]);
                        otherVertices.append(quad[1]);
                        otherVertices.append(quad[2]);
                        otherVertices.append(quad[3]);
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
            if (hiddenVertices.getVertexCount() > 0) {
                window.draw(hiddenVertices);
            }
        }
    }
}

Tile World::getTile(int x, int y) const {
    int chunkX, chunkY, tileX, tileY;
    computeChunkCoords(x, y, chunkX, chunkY, tileX, tileY);
    
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
    Chunk* chunk = nullptr;
    int tileX = 0;
    int tileY = 0;
    if (!locateTileMutable(x, y, chunk, tileX, tileY) || chunk == nullptr) {
        return;
    }

    bool explored = !chunk->tiles[tileX][tileY].fogEnabled;
    chunk->tiles[tileX][tileY] = tile;
    if (explored) {
        chunk->tiles[tileX][tileY].fogEnabled = false;
        chunk->tiles[tileX][tileY].visible = true;
    }
    refreshVisibilityAround(x, y);
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
    
    std::vector<std::pair<int, int>> newChunks;
    
    for (int y = playerChunkY - renderDistance; y <= playerChunkY + renderDistance; ++y) {
        for (int x = playerChunkX - renderDistance; x <= playerChunkX + renderDistance; ++x) {
            if (!chunkExists(x, y)) {
                generateChunk(x, y);
                newChunks.push_back({x, y});
            }
        }
    }
    
    // Generate trees for newly created chunks
    for (const auto& chunkPos : newChunks) {
        generateTrees(chunkPos.first, chunkPos.second);
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

void World::computeChunkCoords(int worldX, int worldY, int& chunkX, int& chunkY, int& tileX, int& tileY) {
    chunkX = worldX / CHUNK_WIDTH;
    chunkY = worldY / CHUNK_HEIGHT;
    tileX = worldX % CHUNK_WIDTH;
    tileY = worldY % CHUNK_HEIGHT;
    if (tileX < 0) {
        tileX += CHUNK_WIDTH;
        --chunkX;
    }
    if (tileY < 0) {
        tileY += CHUNK_HEIGHT;
        --chunkY;
    }
}

bool World::locateTileMutable(int worldX, int worldY, Chunk*& chunkOut, int& tileX, int& tileY) {
    int chunkX = 0;
    int chunkY = 0;
    computeChunkCoords(worldX, worldY, chunkX, chunkY, tileX, tileY);
    chunkOut = getChunk(chunkX, chunkY);
    return chunkOut != nullptr;
}

bool World::isAirTile(int worldX, int worldY) const {
    int chunkX, chunkY, tileX, tileY;
    computeChunkCoords(worldX, worldY, chunkX, chunkY, tileX, tileY);

    for (const Chunk& chunk : chunks) {
        if (chunk.x == chunkX && chunk.y == chunkY) {
            return chunk.tiles[tileX][tileY].type == 0;
        }
    }

    return false;
}

bool World::shouldTileBeVisible(int worldX, int worldY, const Tile& tile) const {
    if (!tile.fogEnabled) {
        return true;
    }

    if (tile.type == 0) {
        return false;
    }

    static const int offsets[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    for (const auto& offset : offsets) {
        if (isAirTile(worldX + offset[0], worldY + offset[1])) {
            return true;
        }
    }
    return false;
}

void World::recalcChunkVisibility(Chunk& chunk) {
    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            int worldX = chunk.x * CHUNK_WIDTH + x;
            int worldY = chunk.y * CHUNK_HEIGHT + y;
            Tile& tile = chunk.tiles[x][y];
            bool newVisible = shouldTileBeVisible(worldX, worldY, tile);
            if (newVisible && tile.fogEnabled) {
                tile.fogEnabled = false;
            }
            tile.visible = newVisible || !tile.fogEnabled;
        }
    }
}

void World::refreshVisibilityAround(int worldX, int worldY) {
    static const int offsets[5][2] = {{0, 0}, {0, -1}, {0, 1}, {-1, 0}, {1, 0}};
    std::vector<Chunk*> dirtyChunks;
    dirtyChunks.reserve(5);

    for (const auto& offset : offsets) {
        int nx = worldX + offset[0];
        int ny = worldY + offset[1];
        Chunk* chunk = nullptr;
        int tileX = 0;
        int tileY = 0;
        if (!locateTileMutable(nx, ny, chunk, tileX, tileY) || chunk == nullptr) {
            continue;
        }

        Tile& tile = chunk->tiles[tileX][tileY];
        bool newVisible = shouldTileBeVisible(nx, ny, tile);
        if (newVisible && tile.fogEnabled) {
            tile.fogEnabled = false;
        }
        tile.visible = newVisible || !tile.fogEnabled;

        if (std::find(dirtyChunks.begin(), dirtyChunks.end(), chunk) == dirtyChunks.end()) {
            dirtyChunks.push_back(chunk);
        }
    }

    for (Chunk* chunk : dirtyChunks) {
        updateChunkVertices(*chunk);
    }
}

void World::refreshNeighborChunks(const Chunk& chunk) {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            Chunk* neighbor = getChunk(chunk.x + dx, chunk.y + dy);
            if (!neighbor) {
                continue;
            }

            recalcChunkVisibility(*neighbor);
            updateChunkVertices(*neighbor);
        }
    }
}
