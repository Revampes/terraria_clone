#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <string>
#include "World.h"
#include "Player.h"
#include "Constants.h"

class Game {
public:
    Game();
    ~Game();
    
    void run();
    
private:
    sf::RenderWindow window;
    World world;
    Player player;
    sf::Clock clock;
    sf::View camera;
    float timeOfDay = 0.0f;
    
    void handleEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(sf::Vector2f mouseWorldPos);
    sf::Vector2f screenToWorld(sf::Vector2i screenPos);
    int worldToTile(float value) const;
    bool withinBreakRange(int tileX, int tileY) const;
    sf::Color computeSkyColor() const;
    float getDaylightFactor() const;
    sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) const;
    void drawHoverOverlay();
    void showWarning(const std::string& message);
    sf::Font uiFont;
    sf::Text hoverText;
    sf::RectangleShape hoverOutline;
    sf::Text warningText;
    bool hoverFontLoaded = false;
    float warningTimer = 0.0f;
};

#endif // GAME_H
