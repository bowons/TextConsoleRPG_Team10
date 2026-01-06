#include "../../include/Unit/Warrior.h"
#include "../../include/Skill/WarriorSkills.h"
#include "../../include/Data/ClassData.h"
#include "../../include/Manager/PrintManager.h"  // 추가

// 기본 생성자 (하드코딩)
Warrior::Warrior(const std::string& name, bool enableInventory)
    : Player(name, enableInventory)
{
    _Stats._MaxHP = 250;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._MaxMP = 100;
    _Stats._CurrentMP = _Stats._MaxMP;
    _Stats._Atk = 35;
    _Stats._Def = 25;
    _Stats._Dex = 15;
    _Stats._Luk = 10;
 _Stats._CriticalRate = 0.08f;
  
    _AggroValue = 30;

    InitializeSkills();
}

// CSV 기반 생성자
Warrior::Warrior(const ClassData& data, const std::string& name, bool enableInventory)
    : Player(data, name, enableInventory)
{
    // 전사는 초기 어그로 30
    _AggroValue = 30;

InitializeSkills();
}

void Warrior::InitializeSkills()
{
    // 워리어 스킬 추가
    AddSkill(std::make_unique<SmashSkill>());  // 강타 (쿨타임 0, 우선순위 높음)
    AddSkill(std::make_unique<RoarSkill>());   // 포효 (쿨타임 3)
}

int Warrior::SelectBestSkill(ICharacter* target) const
{
    // 워리어 스킬 우선순위:
    // 1순위: 포효 (파티원 위험 + MP 충분 + 쿨타임 없음)
    // 2순위: 강타 (HP 70% 이상 + MP 충분 + 쿨타임 없음)
    
    // 포효 (인덱스 1)
    if (CanUseSkill(1))
    {
        return 1;  // 포효의 CanActivate에서 파티원 HP 40% 미만 체크
    }
 
    // 강타 (인덱스 0)
    if (CanUseSkill(0))
    {
        float hpRatio = static_cast<float>(GetCurrentHP()) / GetMaxHP();
    if (hpRatio >= 0.7f)  // HP 70% 이상
   return 0;
  }
    
    return -1;  // 사용 가능한 스킬 없음 → 일반 공격
}

std::string Warrior::GetAggroMaxDialogue() const
{
    return _Name + ": 하하! 나한테 반하기라도 했나? 좀 더 열정적으로 덤벼보라고!";
}

void Warrior::ApplyProficiencyGrowth()
{
    // 전사: HP 숙련도 추가 보너스
    // 기획서: 전투 종료 시 잃은 체력 비례로 추가 방어력 증가
    
    if (_DamageTakenThisBattle >= 100)
    {
    // 피해 100당 방어력 +1
        int defGain = _DamageTakenThisBattle / 100;
        _Stats._Def += defGain;

        PrintManager::GetInstance()->PrintLogLine(
        _Name + "의 방어 기술이 향상되었습니다! (DEF +" + std::to_string(defGain) + ")",
   ELogImportance::DISPLAY
      );
    }
}
