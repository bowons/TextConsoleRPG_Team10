#pragma once
#include "IMonster.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/MonsterSpawnData.h"
#include <tuple>
#include <memory>

class NormalMonster : public IMonster
{
private:
    std::string _AttackName;  // CSV에서 로드한 공격명

public:

    //NormalMonster(const int PlayerLevel, std::string Stage, std::string Name); 
    NormalMonster(const MonsterSpawnData& Data);
    virtual int TakeDamage(ICharacter* Target, int amount) override;
    std::tuple<std::string,int> Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    
    std::tuple<int, int, std::unique_ptr<IItem>> DropReward() override;
    std::string GetAttackNarration() const override;

};
