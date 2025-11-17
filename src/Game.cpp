#include "Game.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <string>
#include <unordered_set>

namespace {
constexpr float DAY_LENGTH_SECONDS = 20.0f * 60.0f;
constexpr float TWO_PI = 6.28318530718f;
constexpr int BREAK_RADIUS = 3;
constexpr const char* DEFAULT_FONT_PATH = "C:/Windows/Fonts/arial.ttf";
constexpr float UI_SLOT_SIZE = 40.0f;
constexpr float UI_SLOT_PADDING = 6.0f;
constexpr float HOTBAR_ORIGIN_X = 12.0f;
constexpr float HOTBAR_ORIGIN_Y = 12.0f;
constexpr float INVENTORY_OFFSET_Y = 20.0f;
constexpr int INVENTORY_COLUMNS = 9;
constexpr int INVENTORY_ROWS = 4;
constexpr float BREAK_BAR_HEIGHT = 4.0f;
constexpr float DROP_TEXT_LIFETIME = 1.5f;
constexpr float DROP_TEXT_SPEED = 18.0f;
constexpr float TOOLTIP_OFFSET_Y = UI_SLOT_SIZE + 16.0f;

std::string tileTypeName(int type) {
    switch (type) {
        case 1: return "Dirt";
        case 2: return "Grass";
        case 3: return "Stone";
        case 4: return "Coal";
        case 5: return "Iron";
        case 6: return "Gold";
        case 7: return "Diamond";
        case 8: return "Wood";
        case 9: return "Leaves";
        case 10: return "Wood Plate";
        default: return "Air";
    }
}
}

Game::Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE), rng(std::random_device{}()) {
    window.setFramerateLimit(60);
    player.setPosition(0, 0);
    
    // Initialize camera
    camera.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.setCenter(player.getPosition());

    hoverOutline.setSize(sf::Vector2f(static_cast<float>(TILE_SIZE), static_cast<float>(TILE_SIZE)));
    hoverOutline.setFillColor(sf::Color::Transparent);
    hoverOutline.setOutlineThickness(1.0f);
    hoverOutline.setOutlineColor(sf::Color::White);

    if (uiFont.loadFromFile(DEFAULT_FONT_PATH)) {
        hoverFontLoaded = true;
        hoverText.setFont(uiFont);
        hoverText.setCharacterSize(12);
        hoverText.setFillColor(sf::Color::White);
        hoverText.setOutlineColor(sf::Color::Black);
        hoverText.setOutlineThickness(1.0f);

        warningText.setFont(uiFont);
        warningText.setCharacterSize(14);
        warningText.setFillColor(sf::Color::Red);
        warningText.setOutlineColor(sf::Color::Black);
        warningText.setOutlineThickness(1.5f);

        inventoryTooltip.setFont(uiFont);
        inventoryTooltip.setCharacterSize(14);
        inventoryTooltip.setFillColor(sf::Color::White);
        inventoryTooltip.setOutlineColor(sf::Color::Black);
        inventoryTooltip.setOutlineThickness(1.0f);
        inventoryTooltip.setPosition(HOTBAR_ORIGIN_X, HOTBAR_ORIGIN_Y + TOOLTIP_OFFSET_Y);

        slotCountText.setFont(uiFont);
        slotCountText.setCharacterSize(12);
        slotCountText.setFillColor(sf::Color::White);
        slotCountText.setOutlineColor(sf::Color::Black);
        slotCountText.setOutlineThickness(1.0f);
    }
    warningText.setString("");

    breakProgressBackground.setSize(sf::Vector2f(static_cast<float>(TILE_SIZE), BREAK_BAR_HEIGHT));
    breakProgressBackground.setFillColor(sf::Color(0, 0, 0, 160));
    breakProgressFill.setSize(sf::Vector2f(0.0f, BREAK_BAR_HEIGHT));
    breakProgressFill.setFillColor(sf::Color(255, 255, 255, 210));

    uiSlotShape.setSize(sf::Vector2f(UI_SLOT_SIZE, UI_SLOT_SIZE));
    uiSlotShape.setFillColor(sf::Color(0, 0, 0, 160));
    uiSlotShape.setOutlineColor(sf::Color(200, 200, 200));
    uiSlotShape.setOutlineThickness(1.0f);

    uiSlotHighlight = uiSlotShape;
    uiSlotHighlight.setOutlineThickness(2.0f);
    uiSlotHighlight.setOutlineColor(sf::Color(255, 255, 120));

    sf::Vector2f inventorySize(
        INVENTORY_COLUMNS * UI_SLOT_SIZE + (INVENTORY_COLUMNS - 1) * UI_SLOT_PADDING + 16.0f,
        INVENTORY_ROWS * UI_SLOT_SIZE + (INVENTORY_ROWS - 1) * UI_SLOT_PADDING + 16.0f
    );
    inventoryPanel.setSize(inventorySize);
    inventoryPanel.setFillColor(sf::Color(0, 0, 0, 180));
    inventoryPanel.setOutlineColor(sf::Color(255, 255, 255, 40));
    inventoryPanel.setOutlineThickness(1.0f);
    inventoryPanel.setPosition(HOTBAR_ORIGIN_X - 8.0f, HOTBAR_ORIGIN_Y + UI_SLOT_SIZE + INVENTORY_OFFSET_Y - 8.0f);

    rebuildSlotBounds();
}

Game::~Game() {}

void Game::run() {
    while (window.isOpen()) {
        sf::Time elapsed = clock.restart();
        float deltaTime = elapsed.asSeconds();
        
        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        else if (event.type == sf::Event::Resized) {
            // Update camera to maintain aspect ratio when window is resized
            float aspectRatio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
            
            // Keep the vertical height constant and adjust horizontal width
            camera.setSize(WINDOW_HEIGHT * aspectRatio, WINDOW_HEIGHT);
            
            // Update viewport to match new window size
            sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
            camera.setViewport(viewport);
            rebuildSlotBounds();
        }
        else if (event.type == sf::Event::KeyPressed) {
            handleInventoryInteractions(event);
        }
        else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2i mousePos(event.mouseButton.x, event.mouseButton.y);
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (handleInventoryClick(mousePos)) {
                    continue;
                }
            }

            if (inventory.isInventoryOpen() && cursorBlocksWorldInput(mousePos)) {
                continue;
            }

            sf::Vector2f worldPos = screenToWorld(mousePos);
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (!inventory.isInventoryOpen()) {
                    beginBreak(worldPos);
                }
            } else if (event.mouseButton.button == sf::Mouse::Right) {
                if (!inventory.isInventoryOpen()) {
                    attemptPlacement(worldPos);
                }
            }
        }
        else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                cancelBreak();
            }
        }
    }
}

void Game::update(float deltaTime) {
    player.update(world, deltaTime);
    updateBreaking(deltaTime);
    updateDropTexts(deltaTime);
    timeOfDay += deltaTime;
    if (timeOfDay >= DAY_LENGTH_SECONDS) {
        timeOfDay = std::fmod(timeOfDay, DAY_LENGTH_SECONDS);
    }
        if (warningTimer > 0.0f) {
            warningTimer = std::max(0.0f, warningTimer - deltaTime);
        }
    
    // Update camera to follow player (keep player centered)
    float centerX = player.getPosition().x + PLAYER_WIDTH / 2;
    float centerY = player.getPosition().y + PLAYER_HEIGHT / 2;
    camera.setCenter(centerX, centerY);
    
    // Generate chunks around player as they move
    int playerChunkX = static_cast<int>(std::floor(player.getPosition().x / (CHUNK_WIDTH * TILE_SIZE)));
    int playerChunkY = static_cast<int>(std::floor(player.getPosition().y / (CHUNK_HEIGHT * TILE_SIZE)));
    
    world.updateChunks(playerChunkX, playerChunkY);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    updateUIHover(mousePos);
}

void Game::render() {
    window.clear(computeSkyColor());
    
    // Set camera view
    window.setView(camera);
    
    world.draw(window, player.getPosition());
    player.draw(window);
    drawHoverOverlay();
    if (breakState.active) {
        float progress = breakState.required > 0.0f ? std::clamp(breakState.elapsed / breakState.required, 0.0f, 1.0f) : 0.0f;
        sf::Vector2f blockOrigin(breakState.tileX * TILE_SIZE, breakState.tileY * TILE_SIZE - 6.0f);
        breakProgressBackground.setPosition(blockOrigin);
        breakProgressFill.setPosition(blockOrigin);
        breakProgressFill.setSize(sf::Vector2f(static_cast<float>(TILE_SIZE) * progress, BREAK_BAR_HEIGHT));
        window.draw(breakProgressBackground);
        window.draw(breakProgressFill);
    }
    drawDropTexts();
        if (warningTimer > 0.0f && hoverFontLoaded) {
            sf::Vector2f playerPos = player.getPosition();
            sf::FloatRect bounds = warningText.getLocalBounds();
            warningText.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
            warningText.setPosition(playerPos.x + PLAYER_WIDTH / 2.0f, playerPos.y - 20.0f);
            window.draw(warningText);
        }

    // Apply darkness overlay based on time of day
    float daylight = getDaylightFactor();
    sf::Uint8 alpha = static_cast<sf::Uint8>(std::clamp(1.0f - daylight, 0.0f, 1.0f) * 140.0f);
    window.setView(window.getDefaultView());
    if (alpha > 0) {
        sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)));
        overlay.setFillColor(sf::Color(0, 0, 0, alpha));
        window.draw(overlay);
    }
    drawUI();
    
    window.display();
}

void Game::handleMouseClick(sf::Vector2f mouseWorldPos) {
    beginBreak(mouseWorldPos);
}

void Game::attemptPlacement(sf::Vector2f mouseWorldPos) {
    int tileX = worldToTile(mouseWorldPos.x);
    int tileY = worldToTile(mouseWorldPos.y);

    if (!withinPlaceRange(tileX, tileY)) {
        showWarning("Too far");
        return;
    }

    Tile target = world.getTile(tileX, tileY);
    if (target.type != 0) {
        showWarning("Occupied");
        return;
    }

    const ItemStack& stack = inventory.selectedHotbarStack();
    if (stack.empty()) {
        showWarning("Empty slot");
        return;
    }

    const ItemDefinition& def = ItemDatabase::definition(stack.id);
    if (!def.placeable || def.tileType == 0) {
        showWarning("Can't place");
        return;
    }

    Tile newTile;
    newTile.type = def.tileType;
    newTile.solid = def.solid;
    newTile.visible = true;
    newTile.fogEnabled = true;

    world.setTile(tileX, tileY, newTile);
    Tile placed = world.getTile(tileX, tileY);
    if (placed.type != newTile.type) {
        showWarning("Can't place here");
        return;
    }

    inventory.consumeFromHotbar(inventory.selectedHotbar(), 1);
}

void Game::beginBreak(sf::Vector2f mouseWorldPos) {
    if (inventory.isInventoryOpen()) {
        return;
    }

    int tileX = worldToTile(mouseWorldPos.x);
    int tileY = worldToTile(mouseWorldPos.y);

    if (!withinBreakRange(tileX, tileY)) {
        showWarning("Too far");
        return;
    }

    Tile targetTile = world.getTile(tileX, tileY);
    if (targetTile.type == 0) {
        showWarning("Nothing here");
        return;
    }
    if (!targetTile.visible) {
        showWarning("Can't see block");
        return;
    }

    breakState.active = true;
    breakState.tileX = tileX;
    breakState.tileY = tileY;
    breakState.elapsed = 0.0f;
    breakState.required = ItemDatabase::breakTimeSeconds(targetTile.type);
    breakState.targetTile = targetTile;
}

void Game::cancelBreak() {
    breakState.active = false;
    breakState.elapsed = 0.0f;
}

void Game::updateBreaking(float deltaTime) {
    if (!breakState.active) {
        return;
    }

    if (inventory.isInventoryOpen()) {
        cancelBreak();
        return;
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        cancelBreak();
        return;
    }

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = screenToWorld(mousePos);
    int tileX = worldToTile(mouseWorld.x);
    int tileY = worldToTile(mouseWorld.y);

    if (tileX != breakState.tileX || tileY != breakState.tileY) {
        cancelBreak();
        return;
    }

    if (!withinBreakRange(tileX, tileY)) {
        cancelBreak();
        return;
    }

    Tile currentTile = world.getTile(tileX, tileY);
    if (currentTile.type == 0) {
        cancelBreak();
        return;
    }

    breakState.targetTile = currentTile;
    breakState.elapsed += deltaTime;
    if (breakState.required > 0.0f && breakState.elapsed >= breakState.required) {
        completeBreak();
    }
}

void Game::completeBreak() {
    if (!breakState.active) {
        return;
    }

    Tile targetTile = breakState.targetTile;
    int tileX = breakState.tileX;
    int tileY = breakState.tileY;
    cancelBreak();

    if (targetTile.type == 0) {
        return;
    }

    sf::Vector2f dropOrigin(tileX * TILE_SIZE + TILE_SIZE / 2.0f, tileY * TILE_SIZE);

    if (targetTile.type == 8) {
        int woodCount = 0;
        auto treeTiles = collectTreeTiles(tileX, tileY, woodCount);
        Tile air;
        air.type = 0;
        air.solid = false;
        air.visible = false;
        air.fogEnabled = true;
        for (const auto& pos : treeTiles) {
            world.setTile(pos.x, pos.y, air);
        }
        auto drops = ItemDatabase::getDropsForTile(8, woodCount, rng);
        grantDrops(drops, dropOrigin);
        return;
    }

    Tile air;
    air.type = 0;
    air.solid = false;
    air.visible = false;
    air.fogEnabled = true;
    world.setTile(tileX, tileY, air);
    auto drops = ItemDatabase::getDropsForTile(targetTile.type, 0, rng);
    grantDrops(drops, dropOrigin);
}

void Game::handleInventoryInteractions(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) {
        return;
    }

    if (event.key.code == sf::Keyboard::Tab) {
        inventory.toggleInventory();
        selectedInventorySlot = -1;
        cancelBreak();
        return;
    }

    if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num9) {
        int hotbarIndex = event.key.code - sf::Keyboard::Num1;
        if (inventory.isInventoryOpen() && selectedInventorySlot != -1) {
            inventory.swapHotbarWithInventory(hotbarIndex, selectedInventorySlot);
        }
        inventory.selectHotbar(hotbarIndex);
    }
}

bool Game::handleInventoryClick(const sf::Vector2i& mousePos) {
    sf::Vector2f mouse = screenToUI(mousePos);

    for (size_t i = 0; i < hotbarSlotBounds.size(); ++i) {
        if (hotbarSlotBounds[i].contains(mouse.x, mouse.y)) {
            inventory.selectHotbar(static_cast<int>(i));
            return true;
        }
    }

    if (!inventory.isInventoryOpen()) {
        return false;
    }

    for (size_t i = 0; i < inventorySlotBounds.size(); ++i) {
        if (inventorySlotBounds[i].contains(mouse.x, mouse.y)) {
            selectedInventorySlot = (selectedInventorySlot == static_cast<int>(i)) ? -1 : static_cast<int>(i);
            return true;
        }
    }

    return false;
}

void Game::updateUIHover(const sf::Vector2i& mousePos) {
    sf::Vector2f mouse = screenToUI(mousePos);
    hoveredHotbarSlot = -1;
    hoveredInventorySlot = -1;

    for (size_t i = 0; i < hotbarSlotBounds.size(); ++i) {
        if (hotbarSlotBounds[i].contains(mouse.x, mouse.y)) {
            hoveredHotbarSlot = static_cast<int>(i);
            break;
        }
    }

    if (inventory.isInventoryOpen()) {
        for (size_t i = 0; i < inventorySlotBounds.size(); ++i) {
            if (inventorySlotBounds[i].contains(mouse.x, mouse.y)) {
                hoveredInventorySlot = static_cast<int>(i);
                break;
            }
        }
    }

    if (!hoverFontLoaded) {
        return;
    }

    const ItemStack* tooltipStack = nullptr;
    if (inventory.isInventoryOpen()) {
        if (hoveredInventorySlot != -1) {
            tooltipStack = &inventory.inventorySlot(hoveredInventorySlot);
        } else if (selectedInventorySlot != -1) {
            tooltipStack = &inventory.inventorySlot(selectedInventorySlot);
        }
    }

    if (!tooltipStack || tooltipStack->empty()) {
        if (hoveredHotbarSlot != -1) {
            tooltipStack = &inventory.hotbarSlot(hoveredHotbarSlot);
        } else {
            tooltipStack = &inventory.selectedHotbarStack();
        }
    }

    if (!tooltipStack || tooltipStack->empty()) {
        inventoryTooltip.setString("");
        return;
    }

    const ItemDefinition& def = ItemDatabase::definition(tooltipStack->id);
    inventoryTooltip.setString(def.name + " x" + std::to_string(tooltipStack->quantity));
}

void Game::drawUI() {
    window.setView(window.getDefaultView());

    auto drawSlot = [&](const sf::FloatRect& bounds, const ItemStack& stack, bool highlight, bool selected) {
        uiSlotShape.setPosition(bounds.left, bounds.top);
        sf::Color fill = highlight ? sf::Color(60, 60, 60, 220) : sf::Color(0, 0, 0, 160);
        uiSlotShape.setFillColor(fill);
        window.draw(uiSlotShape);

        if (selected) {
            uiSlotHighlight.setPosition(bounds.left, bounds.top);
            window.draw(uiSlotHighlight);
        }

        if (!stack.empty()) {
            const ItemDefinition& def = ItemDatabase::definition(stack.id);
            sf::RectangleShape content;
            content.setSize(sf::Vector2f(UI_SLOT_SIZE - 8.0f, UI_SLOT_SIZE - 8.0f));
            content.setFillColor(def.slotColor);
            content.setPosition(bounds.left + 4.0f, bounds.top + 4.0f);
            window.draw(content);

            slotCountText.setString(std::to_string(stack.quantity));
            sf::FloatRect textBounds = slotCountText.getLocalBounds();
            slotCountText.setOrigin(textBounds.left + textBounds.width, textBounds.top + textBounds.height);
            slotCountText.setPosition(bounds.left + UI_SLOT_SIZE - 4.0f, bounds.top + UI_SLOT_SIZE - 2.0f);
            window.draw(slotCountText);
        }
    };

    const auto& hotbar = inventory.hotbarSlots();
    for (size_t i = 0; i < hotbar.size(); ++i) {
        bool highlight = hoveredHotbarSlot == static_cast<int>(i);
        bool selected = inventory.selectedHotbar() == static_cast<int>(i);
        drawSlot(hotbarSlotBounds[i], hotbar[i], highlight, selected);
    }

    if (inventory.isInventoryOpen()) {
        window.draw(inventoryPanel);
        const auto& slots = inventory.inventorySlots();
        for (size_t i = 0; i < slots.size(); ++i) {
            bool highlight = hoveredInventorySlot == static_cast<int>(i);
            bool selected = selectedInventorySlot == static_cast<int>(i);
            drawSlot(inventorySlotBounds[i], slots[i], highlight, selected);
        }
    }

    if (hoverFontLoaded && !inventoryTooltip.getString().isEmpty()) {
        window.draw(inventoryTooltip);
    }
}

void Game::drawDropTexts() {
    if (!hoverFontLoaded) {
        return;
    }
    for (auto& drop : dropTexts) {
        window.draw(drop.text);
    }
}

void Game::updateDropTexts(float deltaTime) {
    for (auto& drop : dropTexts) {
        drop.lifetime -= deltaTime;
        drop.text.move(drop.velocity * deltaTime);
        if (drop.lifetime < 0.5f) {
            sf::Color color = drop.text.getFillColor();
            float t = std::max(drop.lifetime / 0.5f, 0.0f);
            color.a = static_cast<sf::Uint8>(255 * t);
            drop.text.setFillColor(color);
            sf::Color outline = drop.text.getOutlineColor();
            outline.a = color.a;
            drop.text.setOutlineColor(outline);
        }
    }
    dropTexts.erase(std::remove_if(dropTexts.begin(), dropTexts.end(), [](const DropFloatingText& text) {
        return text.lifetime <= 0.0f;
    }), dropTexts.end());
}

bool Game::cursorBlocksWorldInput(const sf::Vector2i& mousePos) const {
    if (!inventory.isInventoryOpen()) {
        return false;
    }
    sf::Vector2f mouse = screenToUI(mousePos);
    for (const auto& rect : hotbarSlotBounds) {
        if (rect.contains(mouse.x, mouse.y)) {
            return true;
        }
    }
    for (const auto& rect : inventorySlotBounds) {
        if (rect.contains(mouse.x, mouse.y)) {
            return true;
        }
    }
    return inventoryPanel.getGlobalBounds().contains(mouse);
}

void Game::rebuildSlotBounds() {
    hotbarSlotBounds.clear();
    inventorySlotBounds.clear();

    for (int i = 0; i < INVENTORY_COLUMNS; ++i) {
        float x = HOTBAR_ORIGIN_X + i * (UI_SLOT_SIZE + UI_SLOT_PADDING);
        float y = HOTBAR_ORIGIN_Y;
        hotbarSlotBounds.emplace_back(x, y, UI_SLOT_SIZE, UI_SLOT_SIZE);
    }

    float inventoryOriginY = HOTBAR_ORIGIN_Y + UI_SLOT_SIZE + INVENTORY_OFFSET_Y;
    for (int row = 0; row < INVENTORY_ROWS; ++row) {
        for (int col = 0; col < INVENTORY_COLUMNS; ++col) {
            float x = HOTBAR_ORIGIN_X + col * (UI_SLOT_SIZE + UI_SLOT_PADDING);
            float y = inventoryOriginY + row * (UI_SLOT_SIZE + UI_SLOT_PADDING);
            inventorySlotBounds.emplace_back(x, y, UI_SLOT_SIZE, UI_SLOT_SIZE);
        }
    }

    float panelX = HOTBAR_ORIGIN_X - 8.0f;
    float panelY = inventoryOriginY - 8.0f;
    inventoryPanel.setPosition(panelX, panelY);
}

std::vector<sf::Vector2i> Game::collectTreeTiles(int startX, int startY, int& woodBlocks) const {
    std::vector<sf::Vector2i> collected;
    woodBlocks = 0;

    if (world.getTile(startX, startY).type != 8) {
        return collected;
    }

    // Flood fill across trunk and leaves so one break removes the entire tree canopy.
    auto encode = [](int x, int y) {
        return (static_cast<long long>(x) << 32) ^ (static_cast<unsigned int>(y));
    };

    std::queue<sf::Vector2i> frontier;
    std::unordered_set<long long> visited;
    frontier.push({startX, startY});
    visited.insert(encode(startX, startY));

    const int MAX_TILES = 512;
    while (!frontier.empty() && static_cast<int>(collected.size()) < MAX_TILES) {
        sf::Vector2i pos = frontier.front();
        frontier.pop();

        Tile tile = world.getTile(pos.x, pos.y);
        if (!ItemDatabase::isTreeTile(tile.type)) {
            continue;
        }

        collected.push_back(pos);
        if (tile.type == 8) {
            ++woodBlocks;
        }

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) {
                    continue;
                }
                int nx = pos.x + dx;
                int ny = pos.y + dy;
                long long key = encode(nx, ny);
                if (visited.insert(key).second) {
                    Tile neighbor = world.getTile(nx, ny);
                    if (ItemDatabase::isTreeTile(neighbor.type)) {
                        frontier.push({nx, ny});
                    }
                }
            }
        }
    }

    return collected;
}

void Game::grantDrops(const std::vector<DropEntry>& drops, const sf::Vector2f& origin) {
    sf::Vector2f textPos = origin;
    for (const auto& drop : drops) {
        if (drop.item == ItemId::None || drop.quantity <= 0) {
            continue;
        }
        bool added = inventory.addItems(drop.item, drop.quantity);
        const ItemDefinition& def = ItemDatabase::definition(drop.item);
        spawnDropText(def.name + " x" + std::to_string(drop.quantity), textPos);
        textPos.y -= 6.0f;
        if (!added) {
            showWarning("Inventory full");
        }
    }
}

void Game::spawnDropText(const std::string& label, const sf::Vector2f& origin) {
    if (!hoverFontLoaded) {
        return;
    }
    DropFloatingText text;
    text.text.setFont(uiFont);
    text.text.setCharacterSize(12);
    text.text.setFillColor(sf::Color::White);
    text.text.setOutlineColor(sf::Color::Black);
    text.text.setOutlineThickness(1.0f);
    text.text.setString(label);
    text.text.setPosition(origin);
    text.velocity = sf::Vector2f(0.0f, -DROP_TEXT_SPEED);
    text.lifetime = DROP_TEXT_LIFETIME;
    dropTexts.push_back(std::move(text));
}

sf::Vector2f Game::screenToWorld(sf::Vector2i screenPos) {
    return window.mapPixelToCoords(screenPos, camera);
}

sf::Vector2f Game::screenToUI(sf::Vector2i screenPos) const {
    return window.mapPixelToCoords(screenPos, window.getDefaultView());
}

int Game::worldToTile(float value) const {
    if (value >= 0) {
        return static_cast<int>(value) / TILE_SIZE;
    }
    return static_cast<int>(value - TILE_SIZE + 1) / TILE_SIZE;
}

bool Game::withinBreakRange(int tileX, int tileY) const {
    sf::Vector2f playerPos = player.getPosition();
    int playerTileX = worldToTile(playerPos.x + PLAYER_WIDTH / 2.0f);
    int playerTileY = worldToTile(playerPos.y + PLAYER_HEIGHT / 2.0f);
    return std::abs(tileX - playerTileX) <= BREAK_RADIUS &&
           std::abs(tileY - playerTileY) <= BREAK_RADIUS;
}

bool Game::withinPlaceRange(int tileX, int tileY) const {
    return withinBreakRange(tileX, tileY);
}

float Game::getDaylightFactor() const {
    float phase = timeOfDay / DAY_LENGTH_SECONDS;
    float wave = 0.5f * (std::sin(TWO_PI * phase - TWO_PI * 0.25f) + 1.0f);
    return std::clamp(wave, 0.0f, 1.0f);
}

sf::Color Game::computeSkyColor() const {
    const sf::Color nightColor(10, 15, 45);
    const sf::Color dayColor(135, 206, 235);
    const sf::Color duskColor(255, 140, 0);

    float daylight = getDaylightFactor();
    sf::Color base = lerpColor(nightColor, dayColor, daylight);

    // Add a warm tint near sunrise/sunset
    float duskBlend = std::sin(std::min(daylight, 1.0f - daylight) * TWO_PI * 0.5f);
    return lerpColor(base, duskColor, std::clamp(duskBlend * 0.3f, 0.0f, 1.0f));
}

sf::Color Game::lerpColor(const sf::Color& a, const sf::Color& b, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);
    auto lerpChannel = [t](sf::Uint8 c1, sf::Uint8 c2) {
        return static_cast<sf::Uint8>(c1 + (c2 - c1) * t);
    };
    return sf::Color(lerpChannel(a.r, b.r),
                     lerpChannel(a.g, b.g),
                     lerpChannel(a.b, b.b),
                     lerpChannel(a.a, b.a));
}

void Game::drawHoverOverlay() {
    if (inventory.isInventoryOpen()) {
        return;
    }
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    if (cursorBlocksWorldInput(mousePos)) {
        return;
    }
    sf::Vector2f worldPos = screenToWorld(mousePos);
    int tileX = worldToTile(worldPos.x);
    int tileY = worldToTile(worldPos.y);

    Tile hoveredTile = world.getTile(tileX, tileY);
    if (hoveredTile.type == 0) {
        return;
    }

    sf::Vector2f tileOrigin(tileX * TILE_SIZE, tileY * TILE_SIZE);
    hoverOutline.setPosition(tileOrigin);

    bool canBreak = hoveredTile.visible && withinBreakRange(tileX, tileY);
    hoverOutline.setOutlineColor(canBreak ? sf::Color::White : sf::Color(200, 200, 200, 160));
    hoverOutline.setOutlineThickness(canBreak ? 1.5f : 1.0f);
    window.draw(hoverOutline);

    if (hoverFontLoaded) {
        hoverText.setString(tileTypeName(hoveredTile.type));
        hoverText.setPosition(tileOrigin.x, tileOrigin.y + TILE_SIZE + 2.0f);
        hoverText.setFillColor(canBreak ? sf::Color::White : sf::Color(200, 200, 200));
        window.draw(hoverText);
    }
}

void Game::showWarning(const std::string& message) {
    if (!hoverFontLoaded) {
        return;
    }
    warningText.setString(message);
    warningTimer = 1.0f;
}
