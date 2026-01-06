#include "../../include/Skill/MonsterSkills.h"
#include "../../include/Unit/Player.h"
#include "../../include/Unit/ICharacter.h"
#include "../../include/Manager/GameManager.h"
#include <random>

// ===== Elite 스킬 구현 =====

SkillResult ElitePowerStrike::CalculateEffect(Player* user, ICharacter* target)
{
    SkillResult result;
    result.SkillName = _Name;
    result.HitCount = 1;
    result.Success = false;

    if (!target)
    {
        result.Message = "대상이 없습니다.";
        return result;
    }

    // Elite의 일반 공격력 × 1.8배 강공격
    int baseDamage = user->GetAtk();
    int damage = static_cast<int>(baseDamage * 1.8f);

    result.Value = damage;
    result.Success = true;
    result.Message = "💥 정예 몬스터의 강력한 일격! 💥";

    return result;
}

bool ElitePowerStrike::CanActivate(const Player* user) const
{
    // Elite는 MP 소모 없음 (항상 사용 가능)
    return true;
}

std::string ElitePowerStrike::GetConditionDescription() const
{
    return "3턴마다 사용 가능";
}

// ===== Boss 페이즈 1 스킬 =====

SkillResult BossPhase1Attack::CalculateEffect(Player* user, ICharacter* target)
{
    SkillResult result;
    result.SkillName = _Name;
    result.HitCount = 1;
    result.Success = false;

    if (!target)
    {
        result.Message = "대상이 없습니다.";
        return result;
    }

    // Boss 공격력 × 1.5배
    int baseDamage = user->GetAtk();
    int damage = static_cast<int>(baseDamage * 1.5f);

    result.Value = damage;
    result.Success = true;
    result.Message = "⚡ 에테르노의 충격파가 대지를 뒤흔든다! ⚡";

    return result;
}

bool BossPhase1Attack::CanActivate(const Player* user) const
{
    return true;
}

std::string BossPhase1Attack::GetConditionDescription() const
{
    return "HP 50% 이상에서 사용";
}

// ===== Boss 페이즈 2 광역 공격 =====

SkillResult BossPhase2AOE::CalculateEffect(Player* user, ICharacter* target)
{
    SkillResult result;
    result.SkillName = _Name;
    result.HitCount = 1;
    result.Success = false;

    // 광역 공격은 target이 nullptr여도 됨 (파티 전체 타격)
    // Boss 공격력 × 1.2배 (광역이라 단일보다 약함)
    int baseDamage = user->GetAtk();
    int damage = static_cast<int>(baseDamage * 1.2f);

    result.Value = damage;
    result.Success = true;
    result.Message = "🌪️ 어둠의 폭풍이 파티 전체를 휩쓴다! 🌪️";

    return result;
}

bool BossPhase2AOE::CanActivate(const Player* user) const
{
    return true;
}

std::string BossPhase2AOE::GetConditionDescription() const
{
    return "HP 50% 미만, 3턴마다 사용";
}

// ===== Boss 디버프 스킬 =====

SkillResult BossDebuff::CalculateEffect(Player* user, ICharacter* target)
{
    SkillResult result;
    result.SkillName = _Name;
    result.HitCount = 1;
    result.Success = true;
    result.Value = 0;  // 디버프는 데미지 없음

    result.Message = "😱 공포의 속삭임이 파티원들의 힘을 약화시킨다! (공격력 -30%, 2라운드)";

    return result;
}

bool BossDebuff::CanActivate(const Player* user) const
{
    return true;
}

std::string BossDebuff::GetConditionDescription() const
{
    return "HP 50% 미만, 4턴마다 사용";
}
