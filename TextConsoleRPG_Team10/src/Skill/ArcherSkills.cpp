#include "../../include/Skill/ArcherSkills.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/SoundPlayer.h"
#include "../../include/Manager/BattleManager.h"

// ===== 정밀 조준 (Aim) =====

AimSkill::AimSkill()
    : ISkill("정밀조준", 10, 2, ESkillType::Buff, ESkillTarget::Self)
{
}

SkillResult AimSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user)
        return SkillResult{ _Name, 0, 0, false, "사용자 없음" };

    // 다음 턴 행운 50% 증가
    int lukBoost = static_cast<int>(user->GetTotalLuk() * 0.5f);
    user->ApplyTempLukBuff(lukBoost, 1);

    // 숙련도 증가
    user->GainAtkProficiency(3);

    // 스킬 사운드 재생
    SoundPlayer::GetInstance()->PlaySFX("Archer_Aim");

    return SkillResult{
        _Name,
        lukBoost,
        1,
        true,
        "1턴간 행운 +" + std::to_string(lukBoost) + " (치명타율 상승)"
    };
}

bool AimSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 전투 시작 첫 턴에만 사용 가능
    BattleManager* bm = BattleManager::GetInstance();
    return bm->GetCurrentRound() == 1;
}

std::string AimSkill::GetConditionDescription() const
{
    return "전투 시작 첫 턴";
}

// ===== 연사 (Multi-Shot) =====

MultiShotSkill::MultiShotSkill()
    : ISkill("연사", 22, 3, ESkillType::Physical, ESkillTarget::SingleEnemy)
{
}

SkillResult MultiShotSkill::CalculateEffect(Player* user, ICharacter* target)
{
    if (!user || !target)
        return SkillResult{ _Name, 0, 0, false, "대상 없음" };

    // 70% ATK로 3회 공격
    int singleHitDamage = static_cast<int>(user->GetTotalAtk() * 0.7f);

    // TODO: 실제로는 BattleManager에서 3번 TakeDamage 호출해야 함
    // 여기서는 총 데미지만 계산
    int totalDamage = singleHitDamage * 3;

    // 숙련도 증가
    user->GainAtkProficiency(7);
    user->GainDexProficiency(5);

    // 스킬 사운드 재생
    SoundPlayer::GetInstance()->PlaySFX("Archer_MultiShot");

    return SkillResult{
        _Name,
        singleHitDamage,  // BattleManager가 HitCount만큼 반복 처리
        3,  // 타격 횟수
        true,
        "3연속 공격! (각 타격마다 치명타 독립 판정)"
    };
}

bool MultiShotSkill::CanActivate(const Player* user) const
{
    if (!user)
        return false;

    // 기본적으로 항상 사용 가능
  // 적 체력 20% 이하 체크는 SelectBestSkill에서 처리
    return true;
}

std::string MultiShotSkill::GetConditionDescription() const
{
    return "적 체력 20% 이하 (마무리용)";
}
