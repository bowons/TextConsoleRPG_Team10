#include "../../include/Skill/WarriorSkills.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/GameManager.h"

// ===== 포효 (Roar) =====

RoarSkill::RoarSkill()
    : ISkill("포효", 12, 3, ESkillType::Buff, ESkillTarget::Self)
{
}

SkillResult RoarSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user)
        return SkillResult{ _Name, 0, 0, false, "사용자 없음" };

    // 3턴간 방어력 50% 증가
    int defBoost = static_cast<int>(user->GetTotalDef() * 0.5f);
    user->ApplyTempDefBuff(defBoost, 3);

    // 어그로 100으로 고정 (3턴간)
    user->LockAggro(100, 3);

    return SkillResult{
        _Name,
        defBoost,
        1,
        true,
        "3턴간 방어력 +" + std::to_string(defBoost) + " + 어그로 100 고정!"
    };
}

bool RoarSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 파티원 중 한 명이라도 HP 40% 미만인가?
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            float hpRatio = static_cast<float>(member->GetCurrentHP()) / member->GetMaxHP();
            if (hpRatio < 0.4f)
                return true;
        }
    }

    return false;
}

std::string RoarSkill::GetConditionDescription() const
{
    return "파티원 중 HP 40% 미만 존재";
}

// ===== 강타 (Smash) =====

SmashSkill::SmashSkill()
    : ISkill("강타", 18, 0, ESkillType::Physical, ESkillTarget::SingleEnemy)
{
}

SkillResult SmashSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user || !target)
        return SkillResult{ _Name, 0, 0, false, "대상 없음" };

    // 100% ATK + 150% DEF
    int damage = user->GetTotalAtk() + static_cast<int>(user->GetTotalDef() * 1.5f);

    // 숙련도 증가
    user->GainAtkProficiency(5);
    user->GainDefProficiency(3);  // 방어력 기반 공격이므로 방어 숙련도도 증가

    return SkillResult{
        _Name,
        damage,
        1,
        true,
        "강력한 일격! (방어력 기반 데미지)"
    };
}

bool SmashSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // HP 70% 이상일 때 사용 가능
    float hpRatio = static_cast<float>(user->GetCurrentHP()) / user->GetMaxHP();
    return hpRatio >= 0.7f;
}

std::string SmashSkill::GetConditionDescription() const
{
    return "자신의 HP 70% 이상";
}
