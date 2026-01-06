#pragma once
#include "ISkill.h"
#include <string>

// ===== Elite 몬스터 스킬 =====

// 강공격 (Elite용 - 3턴마다 사용)
class ElitePowerStrike : public ISkill
{
public:
    ElitePowerStrike()
        : ISkill("강력한 일격", 0, 3, ESkillType::Physical, ESkillTarget::SingleEnemy)
    {
    }

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// ===== Boss 몬스터 스킬 =====

// 보스 페이즈 1 스킬: 단일 강공격
class BossPhase1Attack : public ISkill
{
public:
    BossPhase1Attack()
 : ISkill("에테르 충격파", 0, 0, ESkillType::Magic, ESkillTarget::SingleEnemy)
    {
 }

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// 보스 페이즈 2 스킬: 광역 공격
class BossPhase2AOE : public ISkill
{
public:
    BossPhase2AOE()
        : ISkill("어둠의 폭풍", 0, 3, ESkillType::Magic, ESkillTarget::AllEnemies)
    {
    }

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// 보스 페이즈 2 스킬: 디버프 (공격력 감소)
class BossDebuff : public ISkill
{
public:
    BossDebuff()
: ISkill("공포의 속삭임", 0, 4, ESkillType::Magic, ESkillTarget::AllEnemies)
    {
    }

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};
