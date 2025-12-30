#pragma once
#include "IItem.h"

class AttackUp : public IItem
{
public:
    //생성자 - 공격력 증가 효과 아이템 설정 (이름, 금액, 증가 수치, 보유 가능 개수)
    AttackUp() : IItem("공격력 증가 포션", 100, 10, 5) {};
    void ApplyEffect(Player& p) override;
};
