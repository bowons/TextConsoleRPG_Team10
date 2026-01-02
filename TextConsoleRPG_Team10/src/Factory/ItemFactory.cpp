#include "../../include/Factory/ItemFactory.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
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

    // 새로운 아이템 추가 시 여기에 추가
    // 예시:
    // Register("DefenseUp", []() {
    //     return std::make_unique<DefenseUp>();
    // });
}