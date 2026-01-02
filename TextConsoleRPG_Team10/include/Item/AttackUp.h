#pragma once
#include "IItem.h"

//플레이어 공격력 증가 아이템
class AttackUp : public IItem
{
public:
    //생성자
    //공격력 증가 효과 아이템 설정 (이름, 금액, 증가 수치, 보유 가능 개수)
    AttackUp() : IItem("공격력 증가 포션", 100, 10, 5) {};
    
    void ApplyEffect(Player& Player) override;
    
    // 동일한 타입의 새 AttackUp 인스턴스 생성
    std::unique_ptr<IItem> Clone() const override;
};