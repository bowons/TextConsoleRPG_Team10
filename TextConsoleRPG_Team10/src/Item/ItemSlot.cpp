#include "../../include/Item/ItemSlot.h"
#include "../../include/Item/IItem.h"

//아이템 슬롯이 비어있는지 확인
//아이템 개수가 0 이하이면 빈 슬롯으로 간주
bool ItemSlot::IsEmpty() const
{
    return _Item == nullptr || _Amount <= 0;  // 방어적 검사로 불일치 방지
}

//아이템을 슬롯에 넣음
//실제 추가는 Inventory 클래스에서 처리
bool ItemSlot::SetItem(std::unique_ptr<IItem> Item, int Amount)
{
    if (Item == nullptr || Amount <= 0) {
        // 유효하지 않은 입력이면 슬롯을 완전히 비움
        ClearItem();
        return false;
    }

    _Item = std::move(Item);  // 소유권 이동
    _Amount = Amount;
    return true;
}

//슬롯에 든 아이템 개수 증가
void ItemSlot::AddAmount(int Amount)
{
    if (Amount <= 0) {
        return;
    }

    // 슬롯에 아이템이 없으면 추가하지 않음 — Inventory에서 새 슬롯 생성/설정을 담당해야 함
    if (_Item == nullptr)
    {
        return;
    }

    _Amount += Amount;
}

// 슬롯에 든 아이템 개수 감소
void ItemSlot::RemoveAmount(int Amount)
{
    if (Amount <= 0) return;

    _Amount -= Amount;

    if (_Amount <= 0) {  // 0 이하가 되면 완전히 비움
        ClearItem();
    }
}

//슬롯에 든 아이템 개수 반환
int ItemSlot::GetAmount() const
{
    return _Amount;
}

//슬롯에 있는 아이템 반환
IItem* ItemSlot::GetItem() const
{
    return _Item.get();  // 포인터 반환, 소유권 유지
}

//아이템 슬롯 초기화
void ItemSlot::ClearItem()
{
    _Item.reset();  // unique_ptr 해제
    _Amount = 0;
}