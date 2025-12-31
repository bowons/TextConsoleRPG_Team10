#pragma once
#include <vector>
#include "../Config.h"
#include "ItemSlot.h"

using namespace std;

class IItem;
class Player;

//인벤토리 클래스
class Inventory
{
private:
    vector<ItemSlot> _Slots; //아이템 슬롯 목록
    int _MaxSlots; //최대 슬롯 수

    int FindEmptySlotIndex() const; //빈 슬롯 인덱스 찾기
    int FindItemSlotIndex(IItem* item) const; //특정 아이템이 든 슬롯 인덱스 찾기

public:
    //생성자
    Inventory(int MaxSlots) : _Slots(MaxSlots), _MaxSlots(MaxSlots) {}
    
    int GetItemAmount(IItem* item) const; //특정 아이템의 총 개수 반환
    void UseItem(int slotIndex, Player& p); //아이템 사용
    bool AddItem(IItem* item, int amount); //아이템 인벤토리에 추가
    void RemoveItem(int slotIndex); //아이템 인벤토리에서 제거

    template<typename T>
    int FindFirstSlotIndexOfType() const
    {
        EItemType type = T().GetItemType();  // 정적 호출
        for (size_t i = 0; i < _Slots.size(); ++i) {
            if (_Slots[i].GetItem() && _Slots[i].GetItem()->GetItemType() == type) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
};
