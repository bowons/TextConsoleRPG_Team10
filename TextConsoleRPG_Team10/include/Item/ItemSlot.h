#pragma once

class IItem;

class ItemSlot
{
private:
    IItem* _Item;
    int _Quantity;
    int _MaxStack;

public:
    ItemSlot(IItem* item, int quantity);
    
    bool AddQuantity(int amount);
    bool RemoveQuantity(int amount);
    IItem* GetItem();
    int GetQuantity();
    bool IsFull();
    bool IsEmpty();
};
