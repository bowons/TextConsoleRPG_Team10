#include "../../include/Skill/MageSkills.h"
#include "../../include/Unit/Player.h"

// ===== 화염구 (Fireball) =====

FireballSkill::FireballSkill()
    : ISkill("화염구", 25, 0, ESkillType::Magic, ESkillTarget::SingleEnemy)
{
}

SkillResult FireballSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user || !target)
        return SkillResult{ _Name, 0, 0, false, "대상 없음" };

    // 250% ATK 마법 데미지
    int baseDamage = static_cast<int>(user->GetTotalAtk() * 2.5f);

    // TODO: 마력 폭주 버프 확인 (나중에 구현)
    // 마력 폭주 상태면 2.5배 추가
    // if (user->HasBuff("마력폭주"))
    //     baseDamage = static_cast<int>(baseDamage * 2.5f);

    // 숙련도 증가
    user->GainMagicProficiency(8);
    user->GainMPProficiency(5);

    return SkillResult{
   _Name,
      baseDamage,
      1,
        true,
 "작열하는 화염구!"
    };
}

bool FireballSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 기본적으로 항상 사용 가능 (MP만 있으면)
    return true;
}

std::string FireballSkill::GetConditionDescription() const
{
    return "MP 25 이상";
}

// ===== 마력 폭주 (Overload) =====

OverloadSkill::OverloadSkill()
    : ISkill("마력폭주", 30, 4, ESkillType::Buff, ESkillTarget::Self)
{
}

SkillResult OverloadSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user)
        return SkillResult{ _Name, 0, 0, false, "사용자 없음" };

    // TODO: 마력 폭주 버프 적용 (커스텀 버프 시스템 필요)
    // 현재는 공격력 버프로 대체 (임시)
    int atkBoost = static_cast<int>(user->GetTotalAtk() * 1.5f);
    user->ApplyTempAtkBuff(atkBoost, 1);  // 1턴 동안 적용

    // 숙련도 증가
    user->GainMagicProficiency(10);
    user->GainMPProficiency(8);

    return SkillResult{
        _Name,
        atkBoost,
        1,
        true,
        "마력이 폭주합니다! 다음 턴 마법 데미지 2.5배!"
    };
}

bool OverloadSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // MP 80% 이상일 때만 사용 가능
    float mpRatio = static_cast<float>(user->GetCurrentMP()) / user->GetMaxMP();
    if (mpRatio < 0.8f)
        return false;

    // TODO: 마력 폭주 버프가 이미 있으면 사용 불가
    // if (user->HasBuff("마력폭주"))
    //     return false;

    return true;
}

std::string OverloadSkill::GetConditionDescription() const
{
    return "MP 80% 이상, 마력폭주 버프 없음";
}
