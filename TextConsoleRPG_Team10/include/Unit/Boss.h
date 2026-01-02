#pragma once
#include "IMonster.h"
#include <tuple>
#include <memory>

class Boss : public IMonster
{
public:
    Boss(const int PlayerLevel, string Stage, string Name);
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) const override;
    bool IsDead() const override;
    tuple<int, int, unique_ptr<IItem>> DropReward() override;
};
