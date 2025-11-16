#include "PlayerRenderer.h"
#include <iostream>

std::array<BodyPart, PlayerRenderer::BODY_PARTS_COUNT> PlayerRenderer::bodyParts = {{
    {0, {38, 54}},  // head
    {1, {38, 54}},  // eyeWhite
    {2, {38, 54}},  // eye
    {3, {38, 54}},  // torso
    {7, {38, 54}},  // rightArm
    {7, {38, 54}},  // leftArm
    {10, {38, 54}}, // legs
    {6, {38, 54}},  // clothes
    {11, {38, 54}}, // pants
    {10, {38, 54}}, // rightLeg
    {10, {38, 54}}  // leftLeg
}};

std::array<HairSprite, 20> PlayerRenderer::hairSprites;
bool PlayerRenderer::loaded = false;

void PlayerRenderer::loadAll() {
    if (loaded) return;
    
    for (int i = 0; i < BODY_PARTS_COUNT; i++) {
        std::string filename = "resources/Player_0_" + std::to_string(bodyParts[i].textureId) + ".png";
        if (!bodyParts[i].texture.loadFromFile(filename)) {
            std::cerr << "Failed to load: " << filename << std::endl;
        } else {
            bodyParts[i].texture.setSmooth(false);
        }
    }
    
    for (int i = 0; i < 20; i++) {
        std::string filename = "resources/playerHair/Player_Hair_" + std::to_string(i + 1) + ".png";
        if (!hairSprites[i].texture.loadFromFile(filename)) {
            std::cerr << "Failed to load: " << filename << std::endl;
        } else {
            hairSprites[i].texture.setSmooth(false);
        }
    }
    
    loaded = true;
}

void PlayerRenderer::render(sf::RenderWindow& window, sf::Vector2f pos, PlayerSkin& skin,
                            bool movingRight, PlayerAnimation& animation, float size) {
    
    // Player should be roughly 16 pixels wide and 27 pixels tall (1.7 blocks)
    // Atlas images are 38x54, so we need to scale them down
    float targetWidth = 16.0f * size;
    float targetHeight = 27.0f * size;
    
    // Adjust position: pos is the top-left corner of hitbox
    // Center horizontally and align sprite so feet are at bottom of hitbox
    pos.x += (16.0f - targetWidth) / 2.0f;  // Center horizontally in the 16-pixel hitbox
    pos.y += 32.0f - targetHeight;  // Align feet to bottom of 32-pixel hitbox
    
    // Save original position before any animation offsets
    sf::Vector2f basePos = pos;
    sf::Vector2f headPos = pos;
    
    // if (animation.isFrameUp) {
    //     headPos.y -= 1.0f * size; // Subtle head bob
    // }
    
    auto drawPart = [&](int partIndex, int x, int y, sf::Color color, sf::Vector2f drawPos, bool flip = false) {
        auto& part = bodyParts[partIndex];
        sf::Sprite sprite;
        sprite.setTexture(part.texture);
        sprite.setTextureRect(part.getTextureCoords(x, y, !movingRight));
        sprite.setPosition(drawPos);
        // Scale to make the 38x54 atlas fit into 16x27 pixels
        sprite.setScale(targetWidth / part.atlasSize.x, targetHeight / part.atlasSize.y);
        sprite.setColor(color);
        window.draw(sprite);
    };
    
    auto drawHair = [&](int hairIndex, int x, int y, sf::Color color, sf::Vector2f drawPos) {
        if (hairIndex < 0 || hairIndex >= 20) return;
        auto& hair = hairSprites[hairIndex];
        sf::Sprite sprite;
        sprite.setTexture(hair.texture);
        sprite.setTextureRect(hair.getTextureCoords(x, y, !movingRight));
        sprite.setPosition(drawPos);
        // Scale to make the 38x54 atlas fit into 16x27 pixels
        sprite.setScale(targetWidth / hair.blockSize.x, targetHeight / hair.blockSize.y);
        sprite.setColor(color);
        window.draw(sprite);
    };

    int frame = (animation.state == PlayerAnimation::running) ? animation.headFrame : 0;

    // Draw legs first
    if (skin.hasPants) {
        drawPart(pants, frame, 0, skin.pantsColor, basePos);
    } else {
        drawPart(legs, frame, 0, skin.skinColor, basePos);
    }

    // Draw body and arms
    if (skin.hasClothes) {
        drawPart(clothes, 0, 0, skin.clothesColor, basePos); // Shirt
        drawPart(rightArm, frame, 0, skin.clothesColor, basePos); // Arms with shirt color
    } else {
        drawPart(torso, 0, 0, skin.skinColor, basePos); // Bare chest
        drawPart(rightArm, frame, 0, skin.skinColor, basePos); // Bare arms
    }

    // Draw head on top
    drawPart(head, 0, 0, skin.skinColor, headPos);
    drawPart(eyeWhite, 0, 0, sf::Color::White, headPos);
    drawPart(eye, 0, 0, skin.eyeColor, headPos);
    drawHair(skin.hairType, 0, 0, skin.hairColor, headPos);
}

void PlayerAnimation::update(float deltaTime) {
    if (state == stay) {
        timer = 0.0f;
        headFrame = 0;
        isFrameUp = false;
    } else if (state == running) {
        float runSpeed = 0.08f;
        
        timer += deltaTime;
        
        while (timer >= runSpeed) {
            timer -= runSpeed;
            headFrame = (headFrame + 1) % 14;
        }
        
        if ((headFrame >= 1 && headFrame <= 3) || (headFrame >= 8 && headFrame <= 10)) {
            isFrameUp = true;
        } else {
            isFrameUp = false;
        }
    }
}
