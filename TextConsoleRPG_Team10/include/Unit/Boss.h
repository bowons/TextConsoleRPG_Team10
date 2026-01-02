#pragma once
#include "IMonster.h"
#include <tuple>
#include <memory>

class Boss : public IMonster
{
public:
    Boss(const int PlayerLevel);
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) const override;
    bool IsDead() const override;
    std::tuple<int, int, std::unique_ptr<IItem>> DropReward() override;
};
