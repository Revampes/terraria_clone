#include "Game.h"
#include <cmath>

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE) {
    window.setFramerateLimit(60);
    player.setPosition(0, 0);
    
    // Initialize camera
    camera.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.setCenter(player.getPosition());
}

Game::~Game() {}

void Game::run() {
    while (window.isOpen()) {
        sf::Time elapsed = clock.restart();
        float deltaTime = elapsed.asSeconds();
        
        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            // Update camera to maintain aspect ratio when window is resized
            float aspectRatio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
            
            // Keep the vertical height constant and adjust horizontal width
            camera.setSize(WINDOW_HEIGHT * aspectRatio, WINDOW_HEIGHT);
            
            // Update viewport to match new window size
            sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
            camera.setViewport(viewport);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                sf::Vector2f worldPos = screenToWorld(mousePos);
                handleMouseClick(worldPos);
            }
        }
    }
}

void Game::update(float deltaTime) {
    player.update(world, deltaTime);
    
    // Update camera to follow player (keep player centered)
    // Round to nearest pixel to prevent camera jitter/shaking
    float centerX = std::round(player.getPosition().x + PLAYER_WIDTH / 2);
    float centerY = std::round(player.getPosition().y + PLAYER_HEIGHT / 2);
    camera.setCenter(centerX, centerY);
    
    // Generate chunks around player as they move
    int playerChunkX = static_cast<int>(std::floor(player.getPosition().x / (CHUNK_WIDTH * TILE_SIZE)));
    int playerChunkY = static_cast<int>(std::floor(player.getPosition().y / (CHUNK_HEIGHT * TILE_SIZE)));
    
    world.updateChunks(playerChunkX, playerChunkY);
}

void Game::render() {
    window.clear(sf::Color(135, 206, 235)); // Sky blue background
    
    // Set camera view
    window.setView(camera);
    
    world.draw(window, player.getPosition());
    player.draw(window);
    
    window.display();
}

void Game::handleMouseClick(sf::Vector2f mouseWorldPos) {
    // Convert world position to tile coordinates
    int tileX = static_cast<int>(mouseWorldPos.x) / TILE_SIZE;
    int tileY = static_cast<int>(mouseWorldPos.y) / TILE_SIZE;
    
    // Handle negative coordinates properly
    if (mouseWorldPos.x < 0) tileX = static_cast<int>(mouseWorldPos.x - TILE_SIZE + 1) / TILE_SIZE;
    if (mouseWorldPos.y < 0) tileY = static_cast<int>(mouseWorldPos.y - TILE_SIZE + 1) / TILE_SIZE;
    
    // Break the tile (set it to air)
    Tile airTile;
    airTile.type = 0;
    airTile.solid = false;
    airTile.visible = false;
    
    world.setTile(tileX, tileY, airTile);
}

sf::Vector2f Game::screenToWorld(sf::Vector2i screenPos) {
    return window.mapPixelToCoords(screenPos, camera);
}
