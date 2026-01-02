#pragma once
#include "IMonster.h"
#include "../../include/Item/IItem.h"
#include <tuple>
#include <memory>

class NormalMonster : public IMonster
{
public:
    NormalMonster(const int PlayerLevel); 
    
    virtual void TakeDamage(int amount) override;
    void Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    
    std::tuple<int, int, std::unique_ptr<IItem>> DropReward() override;
    std::string GetAttackNarration() const override;
};
