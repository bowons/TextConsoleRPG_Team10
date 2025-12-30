#pragma once
#include "IMonster.h"

class NormalMonster : public IMonster
{
public:
    NormalMonster(const int PlayerLevel); 
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) override;
    bool IsDead() override;
    void DropReward() override;
};
