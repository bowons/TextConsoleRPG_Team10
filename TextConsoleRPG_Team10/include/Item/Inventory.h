#pragma once
#include <vector>

using namespace std;

class IItem; 
class ItemSlot;
class Player;

//인벤토리 클래스
class Inventory
{
private:
    vector<ItemSlot*> _Slots; //아이템 슬롯 벡터
    int _MaxSlots; //최대 슬롯 수

public:
    Inventory(int maxSlots);
    void UseItem(int slotIndex, Player& p);
    bool AddItem(IItem* item, int quantity);
    void RemoveItem(int slotIndex, int quantity);
    int FindItemSlot(IItem* item);
    int GetEmptySlot();
};
