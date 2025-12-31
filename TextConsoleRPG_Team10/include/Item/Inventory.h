#pragma once
#include <vector>
#include "ItemSlot.h"
#include "HealPotion.h"
#include "AttackUp.h"

using namespace std;

class IItem;
class ItemSlot;
class Player;

//고정 슬롯 인벤토리 클래스
class Inventory
{
private:
    vector<ItemSlot> _Slots; //아이템 슬롯 목록

    //인벤토리 슬롯에 들어갈 아이템들
    HealPotion _HealPotion; //체력 회복 포션 아이템
    AttackUp _AttackUp; //공격력 증가 포션 아이템
        

public:
    //생성자
    Inventory() {
        _Slots.resize(2); //인벤토리 슬롯 2개 고정

        _Slots[0].SetItem(&_HealPotion, 0); //첫 번째 슬롯에 체력 회복 포션 설정
        _Slots[1].SetItem(&_AttackUp, 0); //두 번째 슬롯에 공격력 증가 포션 설정
    }
    
    int GetItemAmount(int SlotIndex) const; //특정 아이템의 총 개수 반환
    bool AddItem(int SlotIndex, int Amount); //아이템 인벤토리에 추가
    bool UseItem(int SlotIndex, Player& P); //아이템 사용
    void RemoveItem(int SlotIndex); //아이템 인벤토리에서 제거 (1개씩 제거 가정)
};
