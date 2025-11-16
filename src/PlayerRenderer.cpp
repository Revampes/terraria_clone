#include "PlayerRenderer.h"
#include <algorithm>
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

    // Target game hitbox is 16x32, sprite atlas is 38x54 per frame.
    float targetWidth = 16.0f * size;
    float targetHeight = 27.0f * size;

    // Align sprite with collision bounds (pos is top-left of hitbox).
    pos.x += (16.0f - targetWidth) / 2.0f;
    pos.y += 32.0f - targetHeight;

    sf::Vector2f basePos = pos;
    sf::Vector2f torsoPos = basePos;
    sf::Vector2f headPos = basePos;

    const float bobOffset = 2.0f * size;

    auto wrapIndex = [](int index, int max) {
        if (max <= 0) return 0;
        int wrapped = index % max;
        if (wrapped < 0) {
            wrapped += max;
        }
        return wrapped;
    };

    auto drawPart = [&](int partIndex, int frameX, int frameY, const sf::Color& color, const sf::Vector2f& drawPos) {
        auto& part = bodyParts[partIndex];
        auto texSize = part.texture.getSize();
        if (texSize.x == 0 || texSize.y == 0) {
            return;
        }

        int atlasWidth = std::max(part.atlasSize.x, 1);
        int atlasHeight = std::max(part.atlasSize.y, 1);
        int columns = std::max(1, static_cast<int>(texSize.x) / atlasWidth);
        int rows = std::max(1, static_cast<int>(texSize.y) / atlasHeight);
        int clampedX = wrapIndex(frameX, columns);
        int clampedY = wrapIndex(frameY, rows);

        sf::Sprite sprite(part.texture);
        sprite.setTextureRect(part.getTextureCoords(clampedX, clampedY, !movingRight));
        sprite.setPosition(drawPos);
        sprite.setScale(targetWidth / static_cast<float>(atlasWidth),
                        targetHeight / static_cast<float>(atlasHeight));
        sprite.setColor(color);
        window.draw(sprite);
    };

    auto drawHair = [&](int hairIndex, int frameX, int frameY, const sf::Color& color, const sf::Vector2f& drawPos) {
        if (hairIndex < 0 || hairIndex >= static_cast<int>(hairSprites.size())) {
            return;
        }

        auto& hair = hairSprites[hairIndex];
        auto texSize = hair.texture.getSize();
        if (texSize.x == 0 || texSize.y == 0) {
            return;
        }

        int blockWidth = std::max(hair.blockSize.x, 1);
        int blockHeight = std::max(hair.blockSize.y, 1);
        int columns = std::max(1, static_cast<int>(texSize.x) / blockWidth);
        int rows = std::max(1, static_cast<int>(texSize.y) / blockHeight);
        int clampedX = wrapIndex(frameX, columns);
        int clampedY = wrapIndex(frameY, rows);

        sf::Sprite sprite(hair.texture);
        sprite.setTextureRect(hair.getTextureCoords(clampedX, clampedY, !movingRight));
        sprite.setPosition(drawPos);
        sprite.setScale(targetWidth / static_cast<float>(blockWidth),
                        targetHeight / static_cast<float>(blockHeight));
        sprite.setColor(color);
        window.draw(sprite);
    };

    int headFrame = animation.headFrame;
    int hairFrame = animation.hairFrame;

    // Head
    drawPart(head, 0, headFrame, skin.skinColor, headPos);
    drawPart(eyeWhite, 0, headFrame, sf::Color::White, headPos);
    drawPart(eye, 0, headFrame, skin.eyeColor, headPos);
    drawHair(std::clamp(skin.hairType, 0, static_cast<int>(hairSprites.size()) - 1), 0, hairFrame, skin.hairColor, headPos);

    if (animation.isFrameUp) {
        torsoPos.y -= bobOffset;
    }

    if (animation.grounded) {
        drawPart(rightArm, animation.handFrameX, 2, skin.skinColor, torsoPos);
    } else {
        drawPart(rightArm, 2, 3, skin.skinColor, torsoPos);
    }

    if (skin.hasClothes) {
        drawPart(clothes, 0, 0, skin.clothesColor, torsoPos);

        if (animation.grounded) {
            drawPart(rightArm, animation.handFrameX, animation.handFrameY, skin.skinColor, torsoPos);
            drawPart(clothes, 0, 3, skin.clothesColor, torsoPos);
        } else {
            drawPart(rightArm, 2, 1, skin.skinColor, torsoPos);
        }
    } else {
        drawPart(torso, 0, 0, skin.skinColor, torsoPos);

        if (animation.grounded) {
            drawPart(rightArm, animation.handFrameX, animation.handFrameY, skin.skinColor, torsoPos);
        } else {
            drawPart(rightArm, 2, 1, skin.skinColor, torsoPos);
        }
    }

    if (skin.hasPants) {
        drawPart(pants, 0, headFrame, skin.pantsColor, basePos);
    } else {
        drawPart(legs, 0, headFrame, skin.skinColor, basePos);
    }
}

void PlayerAnimation::update(float deltaTime) {
    if (state == stay) {
        bool grounded_ = grounded;
        *this = PlayerAnimation{};
        grounded = grounded_;
    } else if (state == running) {
        float runSpeed = 0.08f;

        timer += deltaTime;

        if (headFrame >= 6) {
            headFrame -= 6;
        }

        while (timer >= runSpeed) {
            timer -= runSpeed;
            headFrame++;
        }

        headFrame %= 14;
                
        if ((headFrame >= 1 && headFrame <= 3) || (headFrame >= 8 && headFrame <= 10)) {
            isFrameUp = true;
        } else {
            isFrameUp = false;
        }

        hairFrame = headFrame;

        handFrameX = (headFrame / 2) % 4 + 3;
        handFrameY = 1;

        headFrame += 6;
    }
}
