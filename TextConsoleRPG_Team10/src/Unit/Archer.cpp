#include "../../include/Unit/Archer.h"
#include "../../include/Skill/ArcherSkills.h"
#include "../../include/Data/ClassData.h"

Archer::Archer(const std::string& name, bool enableInventory)
    : Player(name, enableInventory)
{
    // 아처 전용 스탯 설정
    _Stats._MaxHP = 180;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._MaxMP = 120;
    _Stats._CurrentMP = _Stats._MaxMP;
    _Stats._Atk = 30;
    _Stats._Def = 15;
    _Stats._Dex = 30;
    _Stats._Luk = 25;
    _Stats._CriticalRate = 0.15f;  // 높은 기본 치명타율

    // 직업별 스킬 초기화
    InitializeSkills();
}

Archer::Archer(const ClassData& data, const std::string& name, bool enableInventory)
    : Player(data, name, enableInventory)
{
    InitializeSkills();
}

void Archer::InitializeSkills()
{
    // 아처 스킬 추가
    AddSkill(std::make_unique<AimSkill>());        // 정밀조준 (쿨타임 2)
    AddSkill(std::make_unique<MultiShotSkill>());  // 연사 (쿨타임 3)
}

int Archer::SelectBestSkill(ICharacter* target) const
{
    if (!target)
        return -1;

    // 아처 스킬 우선순위:
    // 1순위: 연사 (적 HP 20% 이하)
    // 2순위: 정밀조준 (전투 시작 첫 턴)

    // 연사 (인덱스 1) - 적 HP 20% 이하
    if (CanUseSkill(1))
    {
        float targetHpRatio = static_cast<float>(target->GetCurrentHP()) / target->GetMaxHP();
        if (targetHpRatio <= 0.2f)
            return 1;
    }

    // 정밀조준 (인덱스 0) - 첫 턴에만 사용 가능
    if (CanUseSkill(0))
    {
        return 0;
    }

    return -1;  // 사용 가능한 스킬 없음 → 일반 공격
}

std::string Archer::GetAggroMaxDialogue() const
{
    return _Name + ": 어라 이거 나 위험한가?";
}

void Archer::ApplyProficiencyGrowth()
{
    // 궁수: 민첩/치명타 숙련도 활용
    // 기획서에 따라 이미 ProcessBattleEndProficiency에서 DEX, Crit Rate 증가 완료
    // 추가 효과 없음 (기본 숙련도 성장만으로 충분)
}
