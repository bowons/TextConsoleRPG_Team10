#include "../../include/Factory/ItemFactory.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Item/FairyEssence.h"
#include "../../include/Item/ShieldDefense.h"
#include "../../include/Item/FocusRune.h"
#include "../../include/Item/LuckyCharm.h"
#include "../../include/Item/TitanAwakening.h"
#include "../../include/Manager/PrintManager.h"
#include <iostream>
#include <memory>

ItemFactory::ItemFactory()
{
    RegisterDefaultItems();
}

// 기본 아이템 타입들을 등록함
// 아이템 정보가 추가될 때 마다 아래 함수에 등록 코드를 추가해야 합니다.
void ItemFactory::RegisterDefaultItems()
{
    // Factory<IItem>::Register 사용
    Register("HealPotion", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<HealPotion>();
        });

    Register("AttackUp", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<AttackUp>();
        });

    Register("FairyEssence", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<FairyEssence>();
        });

    Register("ShieldDefense", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<ShieldDefense>();
        });

    Register("FocusRune", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<FocusRune>();
        });

    Register("LuckyCharm", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<LuckyCharm>();
        });

    Register("TitanAwakening", []() -> std::unique_ptr<IItem>
        {
            return std::make_unique<TitanAwakening>();
        });

    // 새로운 아이템 추가 시 여기에 추가
    // 예시:
    // Register("DefenseUp", []() {
    //     return std::make_unique<DefenseUp>();
    // });
}

// CSV 데이터를 기반으로 아이템 생성
std::unique_ptr<IItem> ItemFactory::CreateFromData(const ItemData& itemData)
{
    // 기본 팩토리로 아이템 생성
    auto item = Create(itemData.ItemID);
    
    if (!item)
    {
        return nullptr;
    }

    // CSV 데이터로 모든 속성 업데이트
    item->SetName(itemData.Name);
    item->SetPrice(itemData.Price);
    item->SetEffectAmount(itemData.EffectAmount);
    item->SetMaxCount(itemData.MaxCount);

    return item;
}