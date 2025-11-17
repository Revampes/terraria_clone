#ifndef INVENTORY_H
#define INVENTORY_H

#include <vector>
#include "Items.h"

struct ItemStack {
    ItemId id = ItemId::None;
    int quantity = 0;
    bool empty() const { return id == ItemId::None || quantity <= 0; }
    void clear() {
        id = ItemId::None;
        quantity = 0;
    }
};

class Inventory {
public:
    Inventory();

    bool addItems(ItemId id, int quantity);
    bool consumeFromHotbar(int hotbarIndex, int quantity);
    ItemStack& hotbarSlot(int index);
    const ItemStack& hotbarSlot(int index) const;
    ItemStack& inventorySlot(int index);
    const ItemStack& inventorySlot(int index) const;
    const std::vector<ItemStack>& hotbarSlots() const;
    const std::vector<ItemStack>& inventorySlots() const;

    void selectHotbar(int index);
    int selectedHotbar() const;

    void toggleInventory();
    void setInventoryOpen(bool open);
    bool isInventoryOpen() const;

    void swapHotbarWithInventory(int hotbarIndex, int inventoryIndex);

    ItemStack& selectedHotbarStack();
    const ItemStack& selectedHotbarStack() const;

private:
    std::vector<ItemStack> hotbar;
    std::vector<ItemStack> inventory;
    int selected = 0;
    bool inventoryOpen = false;

    bool addToSlots(std::vector<ItemStack>& slots, ItemId id, int& quantity);
};

#endif // INVENTORY_H
