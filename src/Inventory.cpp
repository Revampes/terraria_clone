#include "Inventory.h"
#include <algorithm>

namespace {
constexpr int HOTBAR_SLOTS = 9;
constexpr int INVENTORY_COLUMNS = 9;
constexpr int INVENTORY_ROWS = 4;
constexpr int INVENTORY_SLOTS = INVENTORY_COLUMNS * INVENTORY_ROWS;
}

Inventory::Inventory() {
    hotbar.resize(HOTBAR_SLOTS);
    inventory.resize(INVENTORY_SLOTS);
}

bool Inventory::addItems(ItemId id, int quantity) {
    if (id == ItemId::None || quantity <= 0) {
        return true;
    }

    int remaining = quantity;
    addToSlots(hotbar, id, remaining);
    addToSlots(inventory, id, remaining);
    return remaining == 0;
}

bool Inventory::addToSlots(std::vector<ItemStack>& slots, ItemId id, int& quantity) {
    if (quantity <= 0) {
        return true;
    }

    const ItemDefinition& def = ItemDatabase::definition(id);

    // Fill existing stacks first
    for (ItemStack& slot : slots) {
        if (slot.id == id && slot.quantity < def.maxStack) {
            int transfer = std::min(def.maxStack - slot.quantity, quantity);
            slot.quantity += transfer;
            quantity -= transfer;
            if (quantity <= 0) {
                return true;
            }
        }
    }

    // Then use empty slots
    for (ItemStack& slot : slots) {
        if (slot.empty()) {
            int transfer = std::min(def.maxStack, quantity);
            slot.id = id;
            slot.quantity = transfer;
            quantity -= transfer;
            if (quantity <= 0) {
                return true;
            }
        }
    }

    return quantity == 0;
}

bool Inventory::consumeFromHotbar(int hotbarIndex, int quantity) {
    if (hotbarIndex < 0 || hotbarIndex >= static_cast<int>(hotbar.size()) || quantity <= 0) {
        return false;
    }

    ItemStack& slot = hotbar[hotbarIndex];
    if (slot.quantity < quantity) {
        return false;
    }

    slot.quantity -= quantity;
    if (slot.quantity == 0) {
        slot.clear();
    }
    return true;
}

ItemStack& Inventory::hotbarSlot(int index) {
    return hotbar.at(index);
}

const ItemStack& Inventory::hotbarSlot(int index) const {
    return hotbar.at(index);
}

ItemStack& Inventory::inventorySlot(int index) {
    return inventory.at(index);
}

const ItemStack& Inventory::inventorySlot(int index) const {
    return inventory.at(index);
}

const std::vector<ItemStack>& Inventory::hotbarSlots() const {
    return hotbar;
}

const std::vector<ItemStack>& Inventory::inventorySlots() const {
    return inventory;
}

void Inventory::selectHotbar(int index) {
    if (index >= 0 && index < static_cast<int>(hotbar.size())) {
        selected = index;
    }
}

int Inventory::selectedHotbar() const {
    return selected;
}

void Inventory::toggleInventory() {
    inventoryOpen = !inventoryOpen;
}

void Inventory::setInventoryOpen(bool open) {
    inventoryOpen = open;
}

bool Inventory::isInventoryOpen() const {
    return inventoryOpen;
}

void Inventory::swapHotbarWithInventory(int hotbarIndex, int inventoryIndex) {
    if (hotbarIndex < 0 || hotbarIndex >= static_cast<int>(hotbar.size())) {
        return;
    }
    if (inventoryIndex < 0 || inventoryIndex >= static_cast<int>(inventory.size())) {
        return;
    }
    std::swap(hotbar[hotbarIndex], inventory[inventoryIndex]);
}

ItemStack& Inventory::selectedHotbarStack() {
    return hotbar.at(selected);
}

const ItemStack& Inventory::selectedHotbarStack() const {
    return hotbar.at(selected);
}
