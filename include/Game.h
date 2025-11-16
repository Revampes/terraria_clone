#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
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
    
    void handleEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(sf::Vector2f mouseWorldPos);
    sf::Vector2f screenToWorld(sf::Vector2i screenPos);
};

#endif // GAME_H
