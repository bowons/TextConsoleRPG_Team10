#pragma once
#include "../Skill/ISkill.h"

// ===== 정밀 조준 (Aim) =====
// 다음 턴 행운 50% 증가
// 조건: 전투 시작 첫 턴
class AimSkill : public ISkill
{
public:
  AimSkill();
    
    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// ===== 연사 (Multi-Shot) =====
// 70% ATK로 3회 공격 (각 타격마다 치명타 독립 계산)
// 조건: 적 체력 20% 이하 (마무리용)
class MultiShotSkill : public ISkill
{
public:
    MultiShotSkill();
    
    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};
