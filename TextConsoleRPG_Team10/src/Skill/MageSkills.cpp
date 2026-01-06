#include "../../include/Skill/MageSkills.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/SoundPlayer.h"
#include "../../include/Unit/Mage.h"

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

    // ===== 마력 폭주 버프 확인 (Mage만 해당) =====
    Mage* mage = dynamic_cast<Mage*>(user);
    if (mage && mage->IsMagicOverloadActive())
    {
        baseDamage = static_cast<int>(baseDamage * 1.5f);  // 1.5배 증폭
    }

    // 숙련도 증가
    user->GainMagicProficiency(8);
    user->GainMPProficiency(5);


    // 스킬 사운드 재생
    SoundPlayer::GetInstance()->PlaySFX("Mage_Fireball");
    std::string message = "작열하는 화염구!";
    if (mage && mage->IsMagicOverloadActive())
    {
        message += " [마력폭주 활성화! 데미지 1.5배!]";
    }

    return SkillResult{
        _Name,
        baseDamage,
   1,
        true,
        message
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

    // ===== 마력 폭주 버프 적용 (2턴 동안) =====
    Mage* mage = dynamic_cast<Mage*>(user);
    if (mage)
    {
     mage->ActivateMagicOverload(2);  // 2턴 동안 유지
    }

    // 숙련도 증가
    user->GainMagicProficiency(10);
    user->GainMPProficiency(8);

    // 스킬 사운드 재생
    SoundPlayer::GetInstance()->PlaySFX("Mage_Overload");

    return SkillResult{
        _Name,
     0,  // 데미지 없음 (버프 스킬)
   1,
        true,
        "마력이 폭주합니다! 2턴간 마법 데미지 1.5배!"
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

    // 마력 폭주 버프가 이미 있으면 사용 불가
    const Mage* mage = dynamic_cast<const Mage*>(user);
    if (mage && mage->IsMagicOverloadActive())
   return false;

    return true;
}

std::string OverloadSkill::GetConditionDescription() const
{
    return "MP 80% 이상, 마력폭주 버프 없음";
}
