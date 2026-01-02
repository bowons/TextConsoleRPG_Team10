#pragma once
#include <memory>
#include "IItem.h"

//아이템 슬롯 클래스
//슬롯은 상태(=아이템 존재 유무 + 아이템 개수)만을 확인
class ItemSlot
{
private:
    std::unique_ptr<IItem> _Item = nullptr; //아이템 포인터 (소유권)
    int _Amount = 0; //슬롯에 든 아이템 개수

public:
    //빈 슬롯 생성자
    ItemSlot() = default;
    
    bool IsEmpty() const; //슬롯이 비어있는지 확인
    bool SetItem(std::unique_ptr<IItem> Item, int Amount); //아이템을 슬롯에 넣음 (아이템+개수 상태, 소유권 이동)
    void AddAmount(int Amount); //슬롯에 든 아이템 개수 증가
    void RemoveAmount(int Amount); //슬롯에 든 아이템 개수 감소
    IItem* GetItem() const; //아이템 반환 (소유권 유지)
    int GetAmount() const; //슬롯에 든 아이템 개수 반환

    void ClearItem(); //아이템 슬롯 초기화
};