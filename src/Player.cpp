#include "Player.h"
#include <algorithm>
#include <cmath>

Player::Player() {
    shape.setSize(sf::Vector2f(PLAYER_WIDTH, PLAYER_HEIGHT));
    shape.setFillColor(sf::Color::Green);
    position = sf::Vector2f(0, 0);
    velocity = sf::Vector2f(0, 0);
    onGround = false;
    movingRight = true;
    
    // Initialize player skin with default values
    skin.hairType = 0;
    
    // Load player renderer resources
    PlayerRenderer::loadAll();
}

Player::~Player() {}

void Player::update(const World& world, float deltaTime) {
    handleInput(deltaTime);
    
    // Update animation state
    animation.grounded = onGround;
    if (std::abs(velocity.x) > 5.0f && onGround) {
        animation.state = PlayerAnimation::running;
    } else {
        animation.state = PlayerAnimation::stay;
    }
    animation.update(deltaTime);
    
    velocity.y += PLAYER_GRAVITY * deltaTime;
    
    sf::Vector2f oldPosition = position;
    bool groundState = onGround;
    position.x += velocity.x * deltaTime;
    if (checkCollisions(world)) {
        position.x = oldPosition.x;
        velocity.x = 0;
    }
    onGround = groundState;
    
    oldPosition = position;
    position.y += velocity.y * deltaTime;
    if (checkCollisions(world)) {
        position.y = oldPosition.y;
        if (velocity.y > 0) {
            onGround = true;
        }
        velocity.y = 0;
    }
    
    // Update shape position
    shape.setPosition(position);
}

void Player::draw(sf::RenderWindow& window) {
    // Use the player renderer instead of simple rectangle
    PlayerRenderer::render(window, position, skin, movingRight, animation, 1.0f);
    
    // Optionally draw the collision box for debugging
    // window.draw(shape);
}

sf::Vector2f Player::getPosition() const {
    return position;
}

void Player::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    shape.setPosition(position);
}

void Player::handleInput(float deltaTime) {
    float desiredDir = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        desiredDir -= 1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        desiredDir += 1.0f;
    }

    if (desiredDir != 0.0f) {
        velocity.x += desiredDir * PLAYER_ACCELERATION * deltaTime;
        velocity.x = std::clamp(velocity.x, -PLAYER_MAX_SPEED, PLAYER_MAX_SPEED);
        movingRight = velocity.x >= 0.0f;
    } else {
        float decel = onGround ? PLAYER_DECELERATION : PLAYER_AIR_DECELERATION;
        if (velocity.x > 0.0f) {
            velocity.x = std::max(0.0f, velocity.x - decel * deltaTime);
        } else if (velocity.x < 0.0f) {
            velocity.x = std::min(0.0f, velocity.x + decel * deltaTime);
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && onGround) {
        velocity.y = -PLAYER_JUMP_VELOCITY;
        onGround = false;
    }
}

bool Player::checkCollisions(const World& world) {
    // Check collisions with tiles - use floor division for negative coordinates
    int playerLeft = static_cast<int>(std::floor(position.x / TILE_SIZE));
    int playerRight = static_cast<int>(std::floor((position.x + PLAYER_WIDTH - 1) / TILE_SIZE));
    int playerTop = static_cast<int>(std::floor(position.y / TILE_SIZE));
    int playerBottom = static_cast<int>(std::floor((position.y + PLAYER_HEIGHT - 1) / TILE_SIZE));
    
    // Reset ground state
    bool wasOnGround = onGround;
    onGround = false;
    
    bool hasCollision = false;
    
    for (int y = playerTop; y <= playerBottom; ++y) {
        for (int x = playerLeft; x <= playerRight; ++x) {
            Tile tile = world.getTile(x, y);
            
            if (tile.solid) {
                sf::FloatRect playerBounds(position.x, position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
                sf::FloatRect tileBounds(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
                
                if (playerBounds.intersects(tileBounds)) {
                    hasCollision = true;
                    
                    // Check if standing on top of this tile
                    float playerBottom = position.y + PLAYER_HEIGHT;
                    float tileTop = y * TILE_SIZE;
                    
                    if (playerBottom > tileTop && playerBottom < tileTop + TILE_SIZE / 2) {
                        onGround = true;
                    }
                }
            }
        }
    }
    
    return hasCollision;
}
