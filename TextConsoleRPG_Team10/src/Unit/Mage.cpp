#include "../../include/Unit/Mage.h"
#include "../../include/Skill/MageSkills.h"
#include "../../include/Data/ClassData.h"
#include "../../include/Manager/PrintManager.h"  // 추가

Mage::Mage(const std::string& name, bool enableInventory)
    : Player(name, enableInventory)
{
    // 메이지 전용 스탯 설정
    _Stats._MaxHP = 150;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._MaxMP = 200;
    _Stats._CurrentMP = _Stats._MaxMP;
    _Stats._Atk = 20;
    _Stats._Def = 10;
    _Stats._Dex = 12;
    _Stats._Luk = 18;
    _Stats._CriticalRate = 0.10f;

    // 직업별 스킬 초기화
    InitializeSkills();
}

Mage::Mage(const ClassData& data, const std::string& name, bool enableInventory)
    : Player(data, name, enableInventory)
{
    InitializeSkills();
}

void Mage::InitializeSkills()
{
    // 메이지 스킬 추가
    AddSkill(std::make_unique<OverloadSkill>());   // 마력폭주 (쿨타임 4)
    AddSkill(std::make_unique<FireballSkill>());   // 화염구 (쿨타임 0)
}

int Mage::SelectBestSkill(ICharacter* target) const
{
    // 메이지 스킬 우선순위:
    // 1순위: 마력폭주 (MP 80% 이상 + 쿨타임 없음)
    // 2순위: 화염구 (항상 사용 가능)

    // 마력폭주 (인덱스 0)
    if (CanUseSkill(0))
    {
        float mpRatio = static_cast<float>(GetCurrentMP()) / GetMaxMP();
        if (mpRatio >= 0.8f)
            return 0;
    }

    // 화염구 (인덱스 1)
    if (CanUseSkill(1))
    {
        return 1;
    }

    return -1;  // 사용 가능한 스킬 없음 → 일반 공격
}

std::string Mage::GetAggroMaxDialogue() const
{
    return _Name + ": 그 기분나쁜 눈 안 치워?";
}

void Mage::ApplyProficiencyGrowth()
{
    // 마법사: MP/지력 숙련도로 공격력 증가
    // 기획서: MP 50 소모당 ATK(마법) +0.5

    int magicProf = GetMagicProficiency();
    if (magicProf > 0)
    {
        // 마법 숙련도 2포인트당 ATK +1 (0.5씩 증가하는 효과)
        int atkGain = magicProf / 2;
        if (atkGain > 0)
        {
            _Stats._Atk += atkGain;
            // 사용한 포인트 차감 (protected 접근 불가하므로 Gain으로 음수 전달)
            GainMagicProficiency(-(atkGain * 2));

            PrintManager::GetInstance()->PrintLogLine(
                _Name + "의 마법 공격력이 증가했습니다! (ATK +" + std::to_string(atkGain) + ")",
                ELogImportance::DISPLAY
            );
        }
    }
}
