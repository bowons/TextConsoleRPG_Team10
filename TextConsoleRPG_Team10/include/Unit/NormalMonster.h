#pragma once
#include "IMonster.h"
#include <tuple>

class NormalMonster : public IMonster
{
public:
    NormalMonster(const int PlayerLevel); 
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) override;
    bool IsDead() override;
    tuple<int, int, unique_ptr<IItem>> DropReward() override;
};
