#ifndef PLAYERRENDERER_H
#define PLAYERRENDERER_H

#include <SFML/Graphics.hpp>
#include <string>
#include <array>

struct PlayerSkin {
    sf::Color skinColor = sf::Color(255, 205, 180);
    sf::Color eyeColor = sf::Color(105, 90, 75);
    sf::Color hairColor = sf::Color(165, 42, 42);
    sf::Color clothesColor = sf::Color(175, 92, 92);
    sf::Color pantsColor = sf::Color(128, 128, 128);
    int hairType = 0;
    bool hasClothes = true;
    bool hasPants = true;
};

struct PlayerAnimation {
    enum State {
        stay,
        running
    } state = stay;
    
    float timer = 0.0f;
    int headFrame = 0;
    int hairFrame = 0;
    int handFrameX = 0;
    int handFrameY = 0;
    bool isFrameUp = false;
    bool grounded = true;
    
    void update(float deltaTime);
};

struct BodyPart {
    int textureId;
    sf::Vector2i atlasSize;
    sf::Texture texture;
    
    sf::IntRect getTextureCoords(int x, int y, bool flip) const {
        int width = atlasSize.x;
        int height = atlasSize.y;
        
        if (flip) {
            return sf::IntRect((x + 1) * width, y * height, -width, height);
        } else {
            return sf::IntRect(x * width, y * height, width, height);
        }
    }
};

struct HairSprite {
    sf::Texture texture;
    sf::Vector2i blockSize = sf::Vector2i(38, 54);
    
    sf::IntRect getTextureCoords(int x, int y, bool flip) const {
        int width = blockSize.x;
        int height = blockSize.y;
        
        if (flip) {
            return sf::IntRect((x + 1) * width, y * height, -width, height);
        } else {
            return sf::IntRect(x * width, y * height, width, height);
        }
    }
};

class PlayerRenderer {
public:
    enum BodyParts {
        head = 0,
        eyeWhite,
        eye,
        torso,
        rightArm,
        leftArm,
        legs,
        clothes,
        pants,
        rightLeg,
        leftLeg,
        BODY_PARTS_COUNT
    };
    
    static void loadAll();
    static void render(sf::RenderWindow& window, sf::Vector2f pos, PlayerSkin& skin, 
                      bool movingRight, PlayerAnimation& animation, float size = 1.0f);
    
private:
    static std::array<BodyPart, BODY_PARTS_COUNT> bodyParts;
    static std::array<HairSprite, 20> hairSprites;
    static bool loaded;
};

#endif // PLAYERRENDERER_H
