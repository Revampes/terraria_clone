#ifndef ITEMS_H
#define ITEMS_H

#include <string>
#include <vector>
#include <random>
#include <SFML/Graphics/Color.hpp>

enum class ItemId {
    None = 0,
    Dirt,
    Grass,
    Stone,
    Coal,
    Iron,
    Gold,
    Diamond,
    Wood,
    WoodPlate,
    Leaf,
    Stick,
    Apple
};

struct ItemDefinition {
    ItemId id = ItemId::None;
    std::string name;
    int maxStack = 100;
    bool placeable = false;
    int tileType = 0;
    bool solid = true;
    sf::Color slotColor = sf::Color::White;
};

struct DropEntry {
    ItemId item = ItemId::None;
    int quantity = 0;
};

class ItemDatabase {
public:
    static const ItemDefinition& definition(ItemId id);
    static ItemId tileToItem(int tileType);
    static int itemToTile(ItemId id);
    static float breakTimeSeconds(int tileType);
    static bool isTreeTile(int tileType);
    static std::vector<DropEntry> getDropsForTile(int tileType, int treeWoodBlocks, std::mt19937& rng);
};

#endif // ITEMS_H
