#pragma once
#include "../Skill/ISkill.h"

// ===== 화염구 (Fireball) =====
// 250% ATK 마법 데미지
// 조건: 적 체력 30% 이상 또는 마력 폭주 상태
class FireballSkill : public ISkill
{
public:
    FireballSkill();

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};

// ===== 마력 폭주 (Overload) =====
// 다음 턴 마법 데미지 2.5배, MP 소모 1.5배
// 조건: MP 80% 이상 + 마력 폭주 버프 없음
class OverloadSkill : public ISkill
{
public:
    OverloadSkill();

    SkillResult CalculateEffect(Player* user, ICharacter* target) override;
    bool CanActivate(const Player* user) const override;
    std::string GetConditionDescription() const override;
};
