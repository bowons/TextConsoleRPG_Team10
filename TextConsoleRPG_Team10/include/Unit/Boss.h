#pragma once
#include "IMonster.h"
#include "../../include/Item/MonsterSpawnData.h"
#include <tuple>
#include <memory>

class Boss : public IMonster
{
public:
    Boss(const MonsterSpawnData& Data);
    virtual int TakeDamage(ICharacter* Target, int amount) override;
    std::tuple<std::string, int> Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    std::tuple<int, int, std::unique_ptr<IItem>> DropReward() override;
    std::string GetAttackNarration() const override;
};
