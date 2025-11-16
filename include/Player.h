#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "World.h"
#include "Constants.h"

class Player {
public:
    Player();
    ~Player();
    
    void update(const World& world, float deltaTime);
    void draw(sf::RenderWindow& window);
    
    sf::Vector2f getPosition() const;
    void setPosition(float x, float y);
    
private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::RectangleShape shape;
    bool onGround;
    
    void handleInput();
    bool checkCollisions(const World& world);
};

#endif // PLAYER_H
