#include "Game.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {
constexpr float DAY_LENGTH_SECONDS = 20.0f * 60.0f;
constexpr float TWO_PI = 6.28318530718f;
constexpr int BREAK_RADIUS = 3;
constexpr const char* DEFAULT_FONT_PATH = "C:/Windows/Fonts/arial.ttf";

std::string tileTypeName(int type) {
    switch (type) {
        case 1: return "Dirt";
        case 2: return "Grass";
        case 3: return "Stone";
        case 4: return "Coal";
        case 5: return "Iron";
        case 6: return "Gold";
        case 7: return "Diamond";
        case 8: return "Wood";
        case 9: return "Leaves";
        default: return "Air";
    }
}
}

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE) {
    window.setFramerateLimit(60);
    player.setPosition(0, 0);
    
    // Initialize camera
    camera.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.setCenter(player.getPosition());

    hoverOutline.setSize(sf::Vector2f(static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)));
    hoverOutline.setFillColor(sf::Color::Transparent);
    hoverOutline.setOutlineThickness(1.0f);
    hoverOutline.setOutlineColor(sf::Color::White);

    if (uiFont.loadFromFile(DEFAULT_FONT_PATH)) {
        hoverFontLoaded = true;
        hoverText.setFont(uiFont);
        hoverText.setCharacterSize(12);
        hoverText.setFillColor(sf::Color::White);
        hoverText.setOutlineColor(sf::Color::Black);
        hoverText.setOutlineThickness(1.0f);
            warningText.setFont(uiFont);
            warningText.setCharacterSize(14);
            warningText.setFillColor(sf::Color::Red);
            warningText.setOutlineColor(sf::Color::Black);
            warningText.setOutlineThickness(1.5f);
    }
        warningText.setString("");
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
    timeOfDay += deltaTime;
    if (timeOfDay >= DAY_LENGTH_SECONDS) {
        timeOfDay = std::fmod(timeOfDay, DAY_LENGTH_SECONDS);
    }
        if (warningTimer > 0.0f) {
            warningTimer = std::max(0.0f, warningTimer - deltaTime);
        }
    
    // Update camera to follow player (keep player centered)
    float centerX = player.getPosition().x + PLAYER_WIDTH / 2;
    float centerY = player.getPosition().y + PLAYER_HEIGHT / 2;
    camera.setCenter(centerX, centerY);
    
    // Generate chunks around player as they move
    int playerChunkX = static_cast<int>(std::floor(player.getPosition().x / (CHUNK_WIDTH * TILE_SIZE)));
    int playerChunkY = static_cast<int>(std::floor(player.getPosition().y / (CHUNK_HEIGHT * TILE_SIZE)));
    
    world.updateChunks(playerChunkX, playerChunkY);
}

void Game::render() {
    window.clear(computeSkyColor());
    
    // Set camera view
    window.setView(camera);
    
    world.draw(window, player.getPosition());
    player.draw(window);
    drawHoverOverlay();
        if (warningTimer > 0.0f && hoverFontLoaded) {
            sf::Vector2f playerPos = player.getPosition();
            sf::FloatRect bounds = warningText.getLocalBounds();
            warningText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
            warningText.setPosition(playerPos.x + PLAYER_WIDTH / 2.0f, playerPos.y - 20.0f);
            window.draw(warningText);
        }

    // Apply darkness overlay based on time of day
    float daylight = getDaylightFactor();
    sf::Uint8 alpha = static_cast<sf::Uint8>(std::clamp(1.0f - daylight, 0.0f, 1.0f) * 140.0f);
    if (alpha > 0) {
        window.setView(window.getDefaultView());
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
        overlay.setFillColor(sf::Color(0, 0, 0, alpha));
        window.draw(overlay);
    }
    
    window.display();
}

void Game::handleMouseClick(sf::Vector2f mouseWorldPos) {
    int tileX = worldToTile(mouseWorldPos.x);
    int tileY = worldToTile(mouseWorldPos.y);

    if (!withinBreakRange(tileX, tileY)) {
        showWarning("Too far");
        return;
    }

    Tile targetTile = world.getTile(tileX, tileY);
    if (targetTile.type == 0) {
        showWarning("Nothing here");
        return;
    }
    if (!targetTile.visible) {
        showWarning("Can't see block");
        return;
    }
    
    // Break the tile (set it to air)
    Tile airTile;
    airTile.type = 0;
    airTile.solid = false;
    airTile.visible = false;
    airTile.fogEnabled = true;
    
    world.setTile(tileX, tileY, airTile);
}

sf::Vector2f Game::screenToWorld(sf::Vector2i screenPos) {
    return window.mapPixelToCoords(screenPos, camera);
}

int Game::worldToTile(float value) const {
    if (value >= 0) {
        return static_cast<int>(value) / TILE_SIZE;
    }
    return static_cast<int>(value - TILE_SIZE + 1) / TILE_SIZE;
}

bool Game::withinBreakRange(int tileX, int tileY) const {
    sf::Vector2f playerPos = player.getPosition();
    int playerTileX = worldToTile(playerPos.x + PLAYER_WIDTH / 2.0f);
    int playerTileY = worldToTile(playerPos.y + PLAYER_HEIGHT / 2.0f);
    return std::abs(tileX - playerTileX) <= BREAK_RADIUS &&
           std::abs(tileY - playerTileY) <= BREAK_RADIUS;
}

float Game::getDaylightFactor() const {
    float phase = timeOfDay / DAY_LENGTH_SECONDS;
    float wave = 0.5f * (std::sin(TWO_PI * phase - TWO_PI * 0.25f) + 1.0f);
    return std::clamp(wave, 0.0f, 1.0f);
}

sf::Color Game::computeSkyColor() const {
    const sf::Color nightColor(10, 15, 45);
    const sf::Color dayColor(135, 206, 235);
    const sf::Color duskColor(255, 140, 0);

    float daylight = getDaylightFactor();
    sf::Color base = lerpColor(nightColor, dayColor, daylight);

    // Add a warm tint near sunrise/sunset
    float duskBlend = std::sin(std::min(daylight, 1.0f - daylight) * TWO_PI * 0.5f);
    return lerpColor(base, duskColor, std::clamp(duskBlend * 0.3f, 0.0f, 1.0f));
}

sf::Color Game::lerpColor(const sf::Color& a, const sf::Color& b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerpChannel = [t](sf::Uint8 c1, sf::Uint8 c2) {
        return static_cast<sf::Uint8>(c1 + (c2 - c1) * t);
    };
    return sf::Color(lerpChannel(a.r, b.r),
                     lerpChannel(a.g, b.g),
                     lerpChannel(a.b, b.b),
                     lerpChannel(a.a, b.a));
}

void Game::drawHoverOverlay() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldPos = screenToWorld(mousePos);
    int tileX = worldToTile(worldPos.x);
    int tileY = worldToTile(worldPos.y);

    Tile hoveredTile = world.getTile(tileX, tileY);
    if (hoveredTile.type == 0) {
        return;
    }

    sf::Vector2f tileOrigin(tileX * TILE_SIZE, tileY * TILE_SIZE);
    hoverOutline.setPosition(tileOrigin);

    bool canBreak = hoveredTile.visible && withinBreakRange(tileX, tileY);
    hoverOutline.setOutlineColor(canBreak ? sf::Color::White : sf::Color(200, 200, 200, 160));
    hoverOutline.setOutlineThickness(canBreak ? 1.5f : 1.0f);
    window.draw(hoverOutline);

    if (hoverFontLoaded) {
        hoverText.setString(tileTypeName(hoveredTile.type));
        hoverText.setPosition(tileOrigin.x, tileOrigin.y + TILE_SIZE + 2.0f);
        hoverText.setFillColor(canBreak ? sf::Color::White : sf::Color(200, 200, 200));
        window.draw(hoverText);
    }
}

void Game::showWarning(const std::string& message) {
    if (!hoverFontLoaded) {
        return;
    }
    warningText.setString(message);
    warningTimer = 1.0f;
}
