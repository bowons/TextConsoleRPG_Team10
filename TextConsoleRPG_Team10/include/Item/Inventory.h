#pragma once
#include <vector>
#include <string>
#include <typeinfo>
#include "ItemSlot.h"

class IItem;
class Player;

//인벤토리 클래스
class Inventory
{
private:
    std::vector<ItemSlot> _Slots; //아이템 슬롯 목록
    int _MaxSlots; //최대 슬롯 수

    int FindEmptySlotIndex() const; //빈 슬롯 인덱스 찾기
    int FindItemSlotIndex(IItem* item) const; //특정 아이템이 든 슬롯 인덱스 찾기

public:
    //생성자
    Inventory(int MaxSlots) : _Slots(MaxSlots), _MaxSlots(MaxSlots) {}

    int GetItemAmount(IItem* item) const; //특정 아이템의 총 개수 반환
    int GetSlotAmount(int SlotIndex) const; // 특정 슬롯의 아이템 개수 반환
    std::string GetSlotItemTypeName(int SlotIndex) const; //특정 슬롯의 아이템 타입 반환
    
    // 특정 슬롯의 아이템 포인터 반환 (조건 체크용)
    // SlotIndex: 슬롯 인덱스
    // return: 아이템 포인터 (없으면 nullptr)
    IItem* GetItemAtSlot(int SlotIndex) const;
 
    // 아이템 사용 가능 여부 체크 (조건 확인)
    // SlotIndex: 슬롯 인덱스
    // player: 사용자
    // currentRound: 현재 라운드
    // return: 사용 가능하면 true
    bool CanUseItem(int SlotIndex, const Player& player, int currentRound) const;

    // 아이템 사용
    // SlotIndex: 슬롯 인덱스
    // p: 사용자
    // return: 사용 성공 시 true
    bool UseItem(int SlotIndex, Player& p);
    
    // 아이템 인벤토리에 추가
    // return: 모두 추가 성공 시 true, 일부만 추가되면 false
    // Remain: 인벤토리에 다 못 넣은 남은 개수(ref out)
    bool AddItem(std::unique_ptr<IItem> Item, int Amount, int& Remain);

    bool RemoveItem(int SlotIndex, int ItemCount = 1); // 해당 아이템 슬롯의 아이템을 ItemCount(Default = 1)만큼 제거
    bool RemoveItemAtSlot(int SlotIndex); // 해당 아이템 슬롯의 아이템을 모두 제거

 template<typename T>
    int FindFirstSlotIndexOfType() const
    {
   for (size_t i = 0; i < _Slots.size(); ++i) {
            IItem* item = _Slots[i].GetItem();
          if (item && typeid(*item) == typeid(T)) { //타입이 완전히 일치한 경우에 index 반환
        return static_cast<int>(i);
}
        }
   return -1;
    }
};
