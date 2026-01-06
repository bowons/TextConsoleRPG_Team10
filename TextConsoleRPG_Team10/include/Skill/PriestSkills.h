#pragma once
#include "../Skill/ISkill.h"

// ===== 치유의 빛 (Heal) =====
// 대상 최대 HP의 20% + 300% ATK 회복
// (언데드 적에게 사용 시 동일 수치 피해)
// 조건: 파티원 중 HP 60% 미만 존재
class HealSkill : public ISkill
{
public:
    HealSkill();

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// ===== 축복 (Bless) =====
// 3턴간 아군 전체 ATK, DEF +20%
// 조건: 아군 전원 HP 80% 이상 (안정적)
class BlessSkill : public ISkill
{
public:
    BlessSkill();

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};
