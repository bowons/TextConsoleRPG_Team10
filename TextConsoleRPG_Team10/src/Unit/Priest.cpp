#include "../../include/Unit/Priest.h"
#include "../../include/Skill/PriestSkills.h"
#include "../../include/Data/ClassData.h"
#include "../../include/Manager/PrintManager.h"

Priest::Priest(const std::string& name, bool enableInventory)
    : Player(name, enableInventory)
{
    // 프리스트 전용 스탯 설정
    _Stats._MaxHP = 170;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._MaxMP = 180;
    _Stats._CurrentMP = _Stats._MaxMP;
    _Stats._Atk = 15;  // 낮은 공격력
    _Stats._Def = 18;
    _Stats._Dex = 14;
    _Stats._Luk = 20;
    _Stats._CriticalRate = 0.06f;
    
    // 직업별 스킬 초기화
    InitializeSkills();
}

Priest::Priest(const ClassData& data, const std::string& name, bool enableInventory)
    : Player(data, name, enableInventory)
{
    // 직업별 스킬 초기화
    InitializeSkills();
}

void Priest::InitializeSkills()
{
    // 프리스트 스킬 추가
    AddSkill(std::make_unique<HealSkill>());   // 치유의빛 (쿨타임 0)
    AddSkill(std::make_unique<BlessSkill>());  // 축복 (쿨타임 5)
}

int Priest::SelectBestSkill(ICharacter* target) const
{
    // 프리스트 스킬 우선순위:
    // 1순위: 치유의빛 (파티원 HP 60% 미만)
    // 2순위: 축복 (파티 전원 HP 80% 이상)
    
    // 치유의빛 (인덱스 0)
    if (CanUseSkill(0))
    {
        return 0;
    }
    
    // 축복 (인덱스 1)
    if (CanUseSkill(1))
    {
        return 1;
    }
    
    return -1;  // 사용 가능한 스킬 없음 → 일반 공격
}

std::string Priest::GetAggroMaxDialogue() const
{
    return _Name + ": 그분의 곁으로 보내드리겠습니다.";
}

void Priest::ApplyProficiencyGrowth()
{
    // 회복 숙련도 성장 (1000 이상 회복 시)
    int magicProf = GetMagicProficiency();
    if (magicProf >= 10)
    {
        // 마법 숙련도 10포인트당 치유의빛 스킬 회복량 +10
        int healBonus = (magicProf / 10) * 10;
        
        const auto& skills = GetSkills();
        if (skills.size() > 0)  // 치유의빛 스킬 (인덱스 0)
        {
            ISkill* healSkill = const_cast<ISkill*>(skills[0].get());
            if (healSkill && healSkill->GetName() == "치유의빛")
            {
                healSkill->AddBonusEffectAmount(healBonus);
                GainMagicProficiency(-(magicProf / 10) * 10);  // 사용한 포인트 차감

                PrintManager::GetInstance()->PrintLogLine(
                    _Name + "의 치유력이 강화되었습니다! (회복량 +" + std::to_string(healBonus) + ")",
                    ELogImportance::DISPLAY
                );
            }
        }
    }
}
