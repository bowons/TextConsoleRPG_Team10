#include "../../include/Item/ItemSlot.h"
#include "../../include/Item/IItem.h"

//아이템 슬롯이 비어있는지 확인
//아이템 개수가 0 이하이면 빈 슬롯으로 간주
bool ItemSlot::IsEmpty() const {
    return _Amount <= 0;
}

//아이템을 슬롯에 넣음
//실제 추가는 Inventory 클래스에서 처리
bool ItemSlot::SetItem(IItem* Item, int Amount) {
    if (Item == nullptr || Amount < 0) {
        return false;
    }

    _Item = Item;
    _Amount = Amount;
    return true;
}

//슬롯에 든 아이템 개수 증가
void ItemSlot::AddAmount(int Amount) {
    if (Amount <= 0) {
        return;
    }
    _Amount += Amount;
}

//슬롯에 든 아이템 개수 감소
void ItemSlot::RemoveAmount(int Amount) {
    if (Amount <= 0) {
        return;
    }
    _Amount -= Amount;

    if (_Amount < 0) {
        _Amount = 0;
    }
}

//슬롯에 든 아이템 개수 반환
int ItemSlot::GetAmount() const{
    return _Amount;
}

//슬롯에 있는 아이템 반환
IItem* ItemSlot::GetItem() const {
    return _Item;
}

//아이템 슬롯 초기화
void ItemSlot::ClearItem() {
    _Amount = 0;
}