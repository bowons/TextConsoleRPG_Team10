#include "../../include/Skill/PriestSkills.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/GameManager.h"

// ===== 치유의 빛 (Heal) =====

HealSkill::HealSkill()
    : ISkill("치유의빛", 20, 0, ESkillType::Heal, ESkillTarget::SingleAlly)
{
}

SkillResult HealSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user || !target)
        return SkillResult{ _Name, 0, 0, false, "대상 없음" };

    // 대상 최대 HP의 20% + 300% ATK + 보너스 회복량
    int healAmount = static_cast<int>(target->GetMaxHP() * 0.2f) +
        static_cast<int>(user->GetTotalAtk() * 3.0f) +
        _BonusEffectAmount;  // 숙련도에 따른 보너스

    // GameManager에서 가장 HP 낮은 아군 찾기
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    Player* lowestHpAlly = nullptr;
    float lowestHpRatio = 1.0f;

    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            float hpRatio = static_cast<float>(member->GetCurrentHP()) / member->GetMaxHP();
            if (hpRatio < lowestHpRatio)
            {
                lowestHpRatio = hpRatio;
                lowestHpAlly = member.get();
            }
        }
    }

    // 가장 HP 낮은 아군 회복
    if (lowestHpAlly)
    {
        lowestHpAlly->ModifyHP(healAmount);

        // 회복량 추적
        user->TrackHealing(healAmount);
    }

    // 숙련도 증가는 제거 (ProcessBattleEndProficiency에서 처리)

    return SkillResult{
        _Name,
        healAmount,
        1,
        true,
        lowestHpAlly ? (lowestHpAlly->GetName() + "의 HP +" + std::to_string(healAmount)) : "대상 없음"
    };
}

bool HealSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 파티원 중 HP 60% 미만인 아군이 있는가?
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            float hpRatio = static_cast<float>(member->GetCurrentHP()) / member->GetMaxHP();
            if (hpRatio < 0.6f)
                return true;
        }
    }

    return false;
}

std::string HealSkill::GetConditionDescription() const
{
    return "파티원 중 HP 60% 미만 존재";
}

// ===== 축복 (Bless) =====

BlessSkill::BlessSkill()
    : ISkill("축복", 35, 5, ESkillType::Buff, ESkillTarget::AllAllies)
{
}

SkillResult BlessSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user)
        return SkillResult{ _Name, 0, 0, false, "사용자 없음" };

    // 파티 전체 버프 적용
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    int buffedCount = 0;
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            int atkBoost = static_cast<int>(member->GetTotalAtk() * 0.2f);
            int defBoost = static_cast<int>(member->GetTotalDef() * 0.2f);

            member->ApplyTempAtkBuff(atkBoost, 3);
            member->ApplyTempDefBuff(defBoost, 3);

            buffedCount++;
        }
    }

    // 숙련도 증가는 제거 (ProcessBattleEndProficiency에서 처리)

    return SkillResult{
        _Name,
        buffedCount,
        1,
        true,
        "파티 전체에 축복이 내려집니다! (3턴간 ATK/DEF +20%)"
    };
}

bool BlessSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 아군 전원의 HP가 80% 이상인가?
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            float hpRatio = static_cast<float>(member->GetCurrentHP()) / member->GetMaxHP();
            if (hpRatio < 0.8f)
                return false;
        }
    }

    return true;
}

std::string BlessSkill::GetConditionDescription() const
{
    return "아군 전원 HP 80% 이상";
}
