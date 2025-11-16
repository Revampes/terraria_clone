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
    lastDeltaTime = 0.0f;
    
    // Initialize player skin with default values
    skin.hairType = 0;
    
    // Load player renderer resources
    PlayerRenderer::loadAll();
}

Player::~Player() {}

void Player::update(const World& world, float deltaTime) {
    lastDeltaTime = deltaTime;
    handleInput();
    
    // Update animation state
    animation.grounded = onGround;
    if (std::abs(velocity.x) > 0.1f && onGround) {
        animation.state = PlayerAnimation::running;
    } else {
        animation.state = PlayerAnimation::stay;
    }
    animation.update(lastDeltaTime);
    
    // Apply gravity only if not on ground
    if (!onGround) {
        velocity.y += PLAYER_GRAVITY;
    } else {
        // When on ground, zero out downward velocity
        if (velocity.y > 0) {
            velocity.y = 0;
        }
    }
    
    // Clamp very small velocities to zero to prevent floating-point drift
    if (std::abs(velocity.x) < 0.01f) velocity.x = 0;
    if (std::abs(velocity.y) < 0.01f && onGround) velocity.y = 0;
    
    // Store old position for collision response
    sf::Vector2f oldPosition = position;
    
    // Update horizontal position
    position.x += velocity.x;
    if (checkCollisions(world)) {
        position.x = oldPosition.x; // Revert if collision
        velocity.x = 0;
    }
    
    // Update vertical position
    position.y += velocity.y;
    if (checkCollisions(world)) {
        position.y = oldPosition.y; // Revert if collision
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

void Player::handleInput() {
    // Horizontal movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        velocity.x = -PLAYER_SPEED;
        movingRight = false;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        velocity.x = PLAYER_SPEED;
        movingRight = true;
    } else {
        velocity.x = 0;
    }
    
    // Jumping
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && onGround) {
        velocity.y = -PLAYER_JUMP_FORCE;
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
