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

    // 동일한 타입의 새 HealPotion 인스턴스 생성
    std::unique_ptr<IItem> Clone() const override;
};