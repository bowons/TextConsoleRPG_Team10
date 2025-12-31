#include "../../include/Item/Inventory.h"
#include "../../include/Item/ItemSlot.h"
#include "../../include/Item/IItem.h"

//특정 아이템의 총 개수 반환
int Inventory::GetItemAmount(int SlotIndex) const {
    if (SlotIndex < 0 || SlotIndex >= _Slots.size())
        return 0;

    return _Slots[SlotIndex].GetAmount();
}

//아이템 추가
bool Inventory::AddItem(int SlotIndex, int Amount) {
    if (Amount <= 0)
        return false; //추가 수량이 0 이하인 경우 false 반환

    if (SlotIndex < 0 || SlotIndex >= _Slots.size())
        return false; //유효하지 않은 슬롯 false 반환

    ItemSlot& Slot = _Slots[SlotIndex];
    IItem* Item = Slot.GetItem();

    if (!Item) {
        return false; //아이템이 없으면 false 반환
    }

    int MaxCount = Item->GetMaxCount(); //아이템 최대 보유 개수
    int Current = Slot.GetAmount(); //현재 슬롯에 든 아이템 개수
    int CanAdd = MaxCount - Current; //추가 가능한 개수 계산

    if (CanAdd <= 0) {
        return false; //더 이상 추가 불가
    }

    int addAmount = min(CanAdd, Amount); //추가할 개수 결정
    Slot.AddAmount(addAmount); //아이템 개수 증가

    return true;
}


//아이템 사용
bool Inventory::UseItem(int SlotIndex, Player& P) {

    if (SlotIndex < 0 || SlotIndex >= _Slots.size())
        return false; //유효하지 않은 슬롯 false 반환

    ItemSlot& slot = _Slots[SlotIndex];

    if (slot.IsEmpty()) {
        return false; //빈 슬롯이면 false 반환
    }

    IItem* item = slot.GetItem();

    if (!item) {
        return false; //아이템이 없으면 false 반환
    }

    item->ApplyEffect(P); //아이템 효과 적용
    slot.RemoveAmount(1); //아이템 개수 1 감소

    return true;

}
void Inventory::RemoveItem(int SlotIndex) {
    if (SlotIndex < 0 || SlotIndex >= _Slots.size()) {
        return; //유효하지 않은 슬롯 false 반환
    }

    ItemSlot& Slot = _Slots[SlotIndex];
    
    if (Slot.IsEmpty()) {
        return; //빈 슬롯이면 작업 수행X
    }

    Slot.RemoveAmount(1); //아이템 개수 1 감소
}