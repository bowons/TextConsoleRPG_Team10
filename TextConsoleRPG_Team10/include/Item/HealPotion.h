#pragma once
#include "IItem.h"

//플레이어 체력 회복 아이템
class HealPotion : public IItem
{
public:
    //생성자
    //체력 회복 아이템 설정 (이름, 금액, 회복 수치, 보유 가능 개수)
    HealPotion() : IItem("체력 회복 포션", 100, 50, 5) {};

    void ApplyEffect(Player& Player) override;
    EItemType GetItemType() const override { return EItemType::HealingPotion; }
};