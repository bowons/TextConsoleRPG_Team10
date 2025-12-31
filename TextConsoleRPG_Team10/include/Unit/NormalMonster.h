#pragma once
#include "IMonster.h"
#include "../../include/Item/IItem.h"
#include <tuple>

class NormalMonster : public IMonster
{
public:
    NormalMonster(const int PlayerLevel); 
    
    virtual void TakeDamage(int amount) override;
    void Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    
    tuple<int, int, IItem*> DropReward() override;
};
