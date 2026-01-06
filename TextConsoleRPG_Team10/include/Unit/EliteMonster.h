#pragma once
#include "IMonster.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Skill/ISkill.h"
#include <tuple>
#include <memory>
#include <vector>

class EliteMonster : public IMonster
{
private:
    // Elite 전용 스킬 시스템
    std::vector<std::unique_ptr<ISkill>> _Skills;
    
    // 턴 카운터 (3턴마다 스킬 사용)
    mutable int _TurnCounter;

    std::string _AttackName;  // CSV에서 로드한 기본 공격명

protected:
    // Elite 스킬 초기화
    void InitializeSkills();

public:
    EliteMonster(const MonsterSpawnData& Data);
 
    int TakeDamage(ICharacter* Target, int amount) override;
    std::tuple<std::string, int> Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    std::tuple<int, int, std::unique_ptr<IItem>> DropReward() override;
    std::string GetAttackNarration() const override;

    // 턴 카운터 증가 (BattleManager에서 호출)
    void IncrementTurnCounter() const { _TurnCounter++; }
    
// 턴 카운터 반환
    int GetTurnCounter() const { return _TurnCounter; }
};
