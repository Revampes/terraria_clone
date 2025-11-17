#include "Items.h"
#include <unordered_map>
#include <algorithm>

namespace {
struct ItemDefinitionRecord {
    ItemDefinition def;
};

const std::unordered_map<ItemId, ItemDefinitionRecord> ITEM_DEFS = {
    {ItemId::None, {{ItemId::None, "Empty", 0, false, 0, false, sf::Color(60, 60, 60)}}},
    {ItemId::Dirt, {{ItemId::Dirt, "Dirt", 100, true, 1, true, sf::Color(139, 69, 19)}}},
    {ItemId::Grass, {{ItemId::Grass, "Grass", 100, true, 2, true, sf::Color(34, 139, 34)}}},
    {ItemId::Stone, {{ItemId::Stone, "Stone", 100, true, 3, true, sf::Color(105, 105, 105)}}},
    {ItemId::Coal, {{ItemId::Coal, "Coal", 100, true, 4, true, sf::Color(54, 69, 79)}}},
    {ItemId::Iron, {{ItemId::Iron, "Iron", 100, true, 5, true, sf::Color(184, 134, 11)}}},
    {ItemId::Gold, {{ItemId::Gold, "Gold", 100, true, 6, true, sf::Color(255, 215, 0)}}},
    {ItemId::Diamond, {{ItemId::Diamond, "Diamond", 100, true, 7, true, sf::Color(0, 191, 255)}}},
    {ItemId::Wood, {{ItemId::Wood, "Wood", 100, true, 8, false, sf::Color(160, 82, 45)}}},
    {ItemId::WoodPlate, {{ItemId::WoodPlate, "Wood Plate", 100, true, 10, true, sf::Color(218, 165, 32)}}},
    {ItemId::Leaf, {{ItemId::Leaf, "Leaf", 100, true, 9, false, sf::Color(34, 139, 34)}}},
    {ItemId::Stick, {{ItemId::Stick, "Stick", 100, false, 0, false, sf::Color(205, 133, 63)}}},
    {ItemId::Apple, {{ItemId::Apple, "Apple", 30, false, 0, false, sf::Color(220, 20, 60)}}}
};

const std::unordered_map<int, ItemId> TILE_TO_ITEM = {
    {1, ItemId::Dirt},
    {2, ItemId::Grass},
    {3, ItemId::Stone},
    {4, ItemId::Coal},
    {5, ItemId::Iron},
    {6, ItemId::Gold},
    {7, ItemId::Diamond},
    {8, ItemId::Wood},
    {10, ItemId::WoodPlate},
    {9, ItemId::Leaf}
};

const std::unordered_map<ItemId, int> ITEM_TO_TILE = {
    {ItemId::Dirt, 1},
    {ItemId::Grass, 2},
    {ItemId::Stone, 3},
    {ItemId::Coal, 4},
    {ItemId::Iron, 5},
    {ItemId::Gold, 6},
    {ItemId::Diamond, 7},
    {ItemId::Wood, 8},
    {ItemId::Leaf, 9},
    {ItemId::WoodPlate, 10}
};

const std::unordered_map<int, float> BREAK_TIMES = {
    {1, 1.0f},
    {2, 1.0f},
    {3, 1.5f},
    {4, 1.7f},
    {5, 2.0f},
    {6, 2.2f},
    {7, 2.5f},
    {8, 3.0f},
    {9, 0.4f},
    {10, 0.6f}
};

constexpr float DEFAULT_BREAK_TIME = 0.6f;
}

const ItemDefinition& ItemDatabase::definition(ItemId id) {
    auto it = ITEM_DEFS.find(id);
    if (it == ITEM_DEFS.end()) {
        return ITEM_DEFS.at(ItemId::None).def;
    }
    return it->second.def;
}

ItemId ItemDatabase::tileToItem(int tileType) {
    auto it = TILE_TO_ITEM.find(tileType);
    if (it == TILE_TO_ITEM.end()) {
        return ItemId::None;
    }
    return it->second;
}

int ItemDatabase::itemToTile(ItemId id) {
    auto it = ITEM_TO_TILE.find(id);
    if (it == ITEM_TO_TILE.end()) {
        return 0;
    }
    return it->second;
}

float ItemDatabase::breakTimeSeconds(int tileType) {
    auto it = BREAK_TIMES.find(tileType);
    if (it == BREAK_TIMES.end()) {
        return DEFAULT_BREAK_TIME;
    }
    return it->second;
}

bool ItemDatabase::isTreeTile(int tileType) {
    return tileType == 8 || tileType == 9;
}

std::vector<DropEntry> ItemDatabase::getDropsForTile(int tileType, int treeWoodBlocks, std::mt19937& rng) {
    std::vector<DropEntry> drops;
    if (tileType == 8) {
        // Tree drop logic
        if (treeWoodBlocks <= 0) {
            treeWoodBlocks = 1;
        }
        std::uniform_int_distribution<int> woodDist(2, 3);
        int totalWood = 0;
        for (int i = 0; i < treeWoodBlocks; ++i) {
            totalWood += woodDist(rng);
        }
        drops.push_back({ItemId::WoodPlate, totalWood});

        std::uniform_int_distribution<int> stickDist(0, 3);
        int sticks = stickDist(rng);
        if (sticks > 0) {
            drops.push_back({ItemId::Stick, sticks});
        }
        std::uniform_int_distribution<int> appleDist(0, 2);
        int apples = appleDist(rng);
        if (apples > 0) {
            drops.push_back({ItemId::Apple, apples});
        }
        return drops;
    }

    ItemId mapped = tileToItem(tileType);
    if (mapped != ItemId::None) {
        drops.push_back({mapped, 1});
    }
    return drops;
}
