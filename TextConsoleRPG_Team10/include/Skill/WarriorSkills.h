#pragma once
#include "../Skill/ISkill.h"

// ===== 포효 (Roar) =====
// 3턴간 자신의 방어력 50% 증가
// 조건: 파티원 중 한 명이라도 HP 40% 미만
class RoarSkill : public ISkill
{
public:
    RoarSkill();
    
    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// ===== 강타 (Smash) =====
// 100% ATK + 150% DEF 데미지
// 조건: 자신의 HP 70% 이상
class SmashSkill : public ISkill
{
public:
    SmashSkill();
    
    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};
