#include "Player.h"
#include <algorithm>
#include <cmath>

Player::Player() {
    shape.setSize(sf::Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
    shape.setFillColor(sf::Color::Green);
    position = sf::Vector2f(0, 0);
    velocity = sf::Vector2f(0, 0);
    onGround = false;
}

Player::~Player() {}

void Player::update(const World& world, float deltaTime) {
    handleInput();
    
    // Apply gravity
    velocity.y += PLAYER_GRAVITY;
    
    // Update horizontal position
    position.x += velocity.x;
    checkCollisions(world);
    
    // Update vertical position
    position.y += velocity.y;
    checkCollisions(world);
    
    // Update shape position
    shape.setPosition(position);
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

sf::Vector2f Player::getPosition() const {
    return position;
}

void Player::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    shape.setPosition(position);
}

void Player::handleInput() {
    // Horizontal movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        velocity.x = -PLAYER_SPEED;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        velocity.x = PLAYER_SPEED;
    } else {
        velocity.x = 0;
    }
    
    // Jumping
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && onGround) {
        velocity.y = -PLAYER_JUMP_FORCE;
        onGround = false;
    }
}

void Player::checkCollisions(const World& world) {
    // Check collisions with tiles - use floor division for negative coordinates
    int playerLeft = static_cast<int>(std::floor(position.x / TILE_SIZE));
    int playerRight = static_cast<int>(std::floor((position.x + PLAYER_WIDTH - 1) / TILE_SIZE));
    int playerTop = static_cast<int>(std::floor(position.y / TILE_SIZE));
    int playerBottom = static_cast<int>(std::floor((position.y + PLAYER_HEIGHT - 1) / TILE_SIZE));
    
    // Reset ground state
    onGround = false;
    
    for (int y = playerTop; y <= playerBottom; ++y) {
        for (int x = playerLeft; x <= playerRight; ++x) {
            Tile tile = world.getTile(x, y);
            
            if (tile.solid) {
                sf::FloatRect playerBounds(position.x, position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
                sf::FloatRect tileBounds(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                
                if (playerBounds.intersects(tileBounds)) {
                    // Calculate overlap on each axis
                    float overlapLeft = playerBounds.left + playerBounds.width - tileBounds.left;
                    float overlapRight = tileBounds.left + tileBounds.width - playerBounds.left;
                    float overlapTop = playerBounds.top + playerBounds.height - tileBounds.top;
                    float overlapBottom = tileBounds.top + tileBounds.height - playerBounds.top;
                    
                    // Find minimum overlap
                    float minOverlapX = std::min(overlapLeft, overlapRight);
                    float minOverlapY = std::min(overlapTop, overlapBottom);
                    
                    // Resolve collision on the axis with smallest overlap
                    if (minOverlapX < minOverlapY) {
                        // Horizontal collision
                        if (overlapLeft < overlapRight) {
                            position.x -= overlapLeft;
                        } else {
                            position.x += overlapRight;
                        }
                        velocity.x = 0;
                    } else {
                        // Vertical collision
                        if (overlapTop < overlapBottom) {
                            position.y -= overlapTop;
                            velocity.y = 0;
                            onGround = true;
                        } else {
                            position.y += overlapBottom;
                            velocity.y = 0;
                        }
                    }
                }
            }
        }
    }
}
