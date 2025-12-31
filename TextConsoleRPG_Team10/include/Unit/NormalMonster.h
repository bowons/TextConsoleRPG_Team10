#pragma once
#include "IMonster.h"
#include "../../include/Item/IItem.h"
#include <tuple>

class NormalMonster : public IMonster
{
public:
    NormalMonster(const int PlayerLevel); 
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) override;
    bool IsDead() override;
    tuple<int, int, IItem*> DropReward() override;
};
