#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include <string>
#include <random>
#include <vector>
#include "World.h"
#include "Player.h"
#include "Constants.h"
#include "Inventory.h"
#include "Items.h"

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    sf::RenderWindow window;
    World world;
    Player player;
    Inventory inventory;
    sf::Clock clock;
    sf::View camera;
    float timeOfDay = 0.0f;
    std::mt19937 rng;

    void handleEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(sf::Vector2f mouseWorldPos);
    void attemptPlacement(sf::Vector2f mouseWorldPos);
    void beginBreak(sf::Vector2f mouseWorldPos);
    void cancelBreak();
    void updateBreaking(float deltaTime);
    void completeBreak();
    void handleInventoryInteractions(const sf::Event& event);
    bool handleInventoryClick(const sf::Vector2i& mousePos);
    void updateUIHover(const sf::Vector2i& mousePos);
    void drawUI();
    void drawDropTexts();
    void updateDropTexts(float deltaTime);
    bool cursorBlocksWorldInput(const sf::Vector2i& mousePos) const;
    void rebuildSlotBounds();
    std::vector<sf::Vector2i> collectTreeTiles(int startX, int startY, int& woodBlocks) const;
    void grantDrops(const std::vector<DropEntry>& drops, const sf::Vector2f& origin);
    void spawnDropText(const std::string& label, const sf::Vector2f& origin);

    sf::Vector2f screenToWorld(sf::Vector2i screenPos);
    sf::Vector2f screenToUI(sf::Vector2i screenPos) const;
    int worldToTile(float value) const;
    bool withinBreakRange(int tileX, int tileY) const;
    bool withinPlaceRange(int tileX, int tileY) const;
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

    struct BreakState {
        bool active = false;
        int tileX = 0;
        int tileY = 0;
        float elapsed = 0.0f;
        float required = 0.0f;
        Tile targetTile;
    } breakState;

    struct DropFloatingText {
        sf::Text text;
        sf::Vector2f velocity;
        float lifetime = 0.0f;
    };

    std::vector<DropFloatingText> dropTexts;
    sf::RectangleShape breakProgressBackground;
    sf::RectangleShape breakProgressFill;
    sf::RectangleShape uiSlotShape;
    sf::RectangleShape uiSlotHighlight;
    sf::RectangleShape inventoryPanel;
    sf::Text inventoryTooltip;
    sf::Text slotCountText;
    std::vector<sf::FloatRect> hotbarSlotBounds;
    std::vector<sf::FloatRect> inventorySlotBounds;
    int hoveredHotbarSlot = -1;
    int hoveredInventorySlot = -1;
    int selectedInventorySlot = -1;
};

#endif // GAME_H
