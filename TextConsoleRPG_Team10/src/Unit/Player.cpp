#include "../../include/Unit/Player.h"
#include "../../include/Data/ClassData.h"
#include <iostream>
#include <cstdlib>
#include "../../include/Manager/PrintManager.h"

// 기본 생성자 (기존 방식 - 하드코딩)
Player::Player(const std::string& Name, bool enableInventory)
{
    _Name = Name;
    _Level = 1;
    _Stats._MaxHP = 200;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._Atk = 30;
    _MaxExp = 100;  // 1레벨 필요 경험치
    _CurrentExp = 0;
    _Gold = 100;

    if (enableInventory)
    {
        _Inventory = std::make_unique<Inventory>(10);
    }

    // 버프 초기화
    _Stats._TempAtk = 0;
    _Stats._TempDef = 0;
    _Stats._TempDex = 0;
    _Stats._TempLuk = 0;
    _Stats._TempCriticalRate = 0.0f;
    _AtkBuffRoundsRemaining = 0;
    _DefBuffRoundsRemaining = 0;
    _DexBuffRoundsRemaining = 0;
    _LukBuffRoundsRemaining = 0;
    _CriticalRateBuffRoundsRemaining = 0;

    // 어그로 초기화 (전투 시작 시 ResetAggro에서 직업별로 설정)
    _AggroValue = 0;
    _IsAggroLocked = false;
    _AggroLockRoundsRemaining = 0;
    _LockedAggroValue = 0;

    // 숙련도 초기화
    _HPProficiency = 0;
    _MPProficiency = 0;
    _AtkProficiency = 0;
    _DefProficiency = 0;
    _DexProficiency = 0;
    _MagicProficiency = 0;

    // 전투 추적 변수 초기화
    _DamageTakenThisBattle = 0;
    _MPSpentThisBattle = 0;
    _CriticalHitsThisBattle = 0;
    _HealingDoneThisBattle = 0;
}

// CSV 기반 생성자
Player::Player(const ClassData& data, const std::string& playerName, bool enableInventory)
{
    _Name = playerName;
    _Level = 1;

    // CSV 데이터로 스탯 초기화
    ApplyClassData(data);

    _MaxExp = 100;  // 1레벨 필요 경험치
    _CurrentExp = 0;
    _Gold = 100;

    if (enableInventory)
    {
        _Inventory = std::make_unique<Inventory>(10);
    }

    // 버프 초기화
    _Stats._TempAtk = 0;
    _Stats._TempDef = 0;
    _Stats._TempDex = 0;
    _Stats._TempLuk = 0;
    _Stats._TempCriticalRate = 0.0f;
    _AtkBuffRoundsRemaining = 0;
    _DefBuffRoundsRemaining = 0;
    _DexBuffRoundsRemaining = 0;
    _LukBuffRoundsRemaining = 0;
    _CriticalRateBuffRoundsRemaining = 0;

    // 어그로 초기화 (전투 시작 시 ResetAggro에서 직업별로 설정)
    _AggroValue = 0;
    _IsAggroLocked = false;
    _AggroLockRoundsRemaining = 0;
    _LockedAggroValue = 0;

    // 숙련도 초기화
    _HPProficiency = 0;
    _MPProficiency = 0;
    _AtkProficiency = 0;
    _DefProficiency = 0;
    _DexProficiency = 0;
    _MagicProficiency = 0;

    // 전투 추적 변수 초기화
    _DamageTakenThisBattle = 0;
    _MPSpentThisBattle = 0;
    _CriticalHitsThisBattle = 0;
    _HealingDoneThisBattle = 0;
}

// CSV 데이터 적용
void Player::ApplyClassData(const ClassData& data)
{
    _Stats._MaxHP = data._HP;
    _Stats._CurrentHP = data._HP;
    _Stats._MaxMP = data._MP;
    _Stats._CurrentMP = data._MP;
    _Stats._Atk = data._Atk;
    _Stats._Def = data._Def;
    _Stats._Dex = data._Dex;
    _Stats._Luk = data._Luk;
    _Stats._CriticalRate = data._CriticalRate;
}

int Player::TakeDamage(ICharacter* Attacker, const int Amount)
{
    _Stats._CurrentHP -= Amount;
    if (_Stats._CurrentHP < 0)
    {
        _Stats._CurrentHP = 0;
    }

    // 숙련도 추적 (HP 성장용)
    TrackDamageTaken(Amount);

    return Amount;
}

std::tuple<std::string, int> Player::Attack(ICharacter* Target) const
{
    if (!Target)
    {
    return std::make_tuple("", 0);
    }

    // ===== 치명타 판정 (LUK 반영) =====
    // 치명타율 = 기본 치명타율 + (총 LUK * 0.1%)
    int totalLuk = GetTotalLuk();
    float lukBonus = totalLuk * 0.001f;  // LUK 1당 0.1% (0.001)
    float totalCritRate = GetTotalCriticalRate() + lukBonus;
    
    // 확률 계산 (0~100 사이)
    int critRoll = std::rand() % 100 + 1;
    float critThreshold = totalCritRate * 100.0f;
    
    // 버프 포함한 총 공격력
    int baseDamage = _Stats._Atk + _Stats._TempAtk;
    
    if (critRoll <= static_cast<int>(critThreshold))
    {
        // ===== 치명타 발동! =====
        int critDamage = baseDamage * 2;
        
        // 치명타 추적 (숙련도 시스템용)
    const_cast<Player*>(this)->TrackCriticalHit();
      
        // 어그로 증가 (치명타 시 +15, 일반 공격보다 높음)
    const_cast<Player*>(this)->ModifyAggro(15);
 
        int actualDamage = Target->TakeDamage(
            const_cast<ICharacter*>(static_cast<const ICharacter*>(this)),
            critDamage);
        
        return std::make_tuple("치명타!", actualDamage);
    }
    
    // ===== 일반 공격 =====
    // 어그로 증가 (일반 공격 시 +10)
    const_cast<Player*>(this)->ModifyAggro(10);
    
    int actualDamage = Target->TakeDamage(
        const_cast<ICharacter*>(static_cast<const ICharacter*>(this)),
        baseDamage);
    
    return std::make_tuple("일반 공격", actualDamage);
}

std::string Player::GetAttackNarration() const
{
    return _Name + "이(가) 용감하게 공격을 날립니다!";
}

bool Player::IsDead() const
{
    return _Stats._CurrentHP <= 0;
}

void Player::CheckLevelUp()
{
    while (_CurrentExp >= _MaxExp)
    {
        _CurrentExp -= _MaxExp;  // 기획서: 초과 경험치 이월
        ProcessLevelUp();
    }
}

void Player::ProcessLevelUp()
{
    // 최대 레벨 10
    if (_Level >= 10)
    {
        // 최대 레벨 도달 시 경험치 더 이상 쌓이지 않도록
        _CurrentExp = 0;
        return;
    }

    _Level++;

    // 기획서: Max HP = Current Max HP + (Next Level × 20)
    _Stats._MaxHP += (_Level * 20);

    // 기획서: ATK = Current ATK + (Next Level × 5)
    _Stats._Atk += (_Level * 5);

    // HP/MP 풀 회복
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._CurrentMP = _Stats._MaxMP;

    // 다음 레벨 필요 경험치 계산
    // 기획서: 2레벨부터 100 + (레벨 × 40)
    if (_Level >= 2)
    {
        _MaxExp = 100 + (_Level * 40);
    }
    else
    {
        _MaxExp = 100;
    }
}

void Player::GainExp(const int Amount)
{
    _CurrentExp += Amount;
    CheckLevelUp();
}

void Player::GainGold(const int Amount)
{
    ModifyGold(Amount);
}

bool Player::TryGetInventory(Inventory*& outInventory)
{
    outInventory = _Inventory.get();
    return outInventory != nullptr;
}

bool Player::UseItem(const int SlotIndex)
{
    Inventory* inventory = nullptr;
    if (!TryGetInventory(inventory))
    {
        return false;
    }

    return inventory->UseItem(SlotIndex, *this);
}

// 범용 스탯 수정 메서드들
void Player::ModifyHP(const int Amount)
{
    _Stats._CurrentHP += Amount;

    if (_Stats._CurrentHP > _Stats._MaxHP)
    {
        _Stats._CurrentHP = _Stats._MaxHP;
    }

    if (_Stats._CurrentHP < 0)
    {
        _Stats._CurrentHP = 0;
    }
}

void Player::ModifyMaxHP(const int Amount)
{
    _Stats._MaxHP += Amount;

    if (_Stats._CurrentHP > _Stats._MaxHP)
    {
        _Stats._CurrentHP = _Stats._MaxHP;
    }
}

void Player::ModifyMP(const int Amount)
{
    _Stats._CurrentMP += Amount;

    if (_Stats._CurrentMP > _Stats._MaxMP)
    {
        _Stats._CurrentMP = _Stats._MaxMP;
    }

    if (_Stats._CurrentMP < 0)
    {
        _Stats._CurrentMP = 0;
    }
}

void Player::ModifyMaxMP(const int Amount)
{
    _Stats._MaxMP += Amount;

    if (_Stats._CurrentMP > _Stats._MaxMP)
    {
        _Stats._CurrentMP = _Stats._MaxMP;
    }
}

void Player::ModifyAtk(const int Amount)
{
    _Stats._Atk += Amount;

    if (_Stats._Atk < 0)
    {
        _Stats._Atk = 0;
    }
}

void Player::ModifyDef(const int Amount)
{
    _Stats._Def += Amount;

    if (_Stats._Def < 0)
    {
        _Stats._Def = 0;
    }
}

void Player::ModifyDex(const int Amount)
{
    _Stats._Dex += Amount;

    if (_Stats._Dex < 0)
    {
        _Stats._Dex = 0;
    }
}

void Player::ModifyLuk(const int Amount)
{
    _Stats._Luk += Amount;

    if (_Stats._Luk < 0)
    {
        _Stats._Luk = 0;
    }
}

void Player::ModifyCriticalRate(const float Amount)
{
    _Stats._CriticalRate += Amount;

    if (_Stats._CriticalRate < 0.0f)
    {
        _Stats._CriticalRate = 0.0f;
    }
}

void Player::ModifyGold(const int Amount)
{
    _Gold += Amount;

    if (_Gold < 0)
    {
        _Gold = 0;
    }
}

// 버프 관리 메서드들
void Player::ApplyTempAtkBuff(const int Amount, const int Rounds)
{
    _Stats._TempAtk += Amount;
    _AtkBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempDefBuff(const int Amount, const int Rounds)
{
    _Stats._TempDef += Amount;
    _DefBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempDexBuff(const int Amount, const int Rounds)
{
    _Stats._TempDex += Amount;
    _DexBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempLukBuff(const int Amount, const int Rounds)
{
    _Stats._TempLuk += Amount;
    _LukBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempCriticalRateBuff(const float Amount, const int Rounds)
{
    _Stats._TempCriticalRate += Amount;
    _CriticalRateBuffRoundsRemaining = Rounds;
}

void Player::ProcessRoundEnd()
{
    // 공격력 버프 처리
    if (_AtkBuffRoundsRemaining > 0)
    {
        _AtkBuffRoundsRemaining--;
        if (_AtkBuffRoundsRemaining == 0)
        {
            _Stats._TempAtk = 0;
        }
    }

    // 방어력 버프 처리
    if (_DefBuffRoundsRemaining > 0)
    {
        _DefBuffRoundsRemaining--;
        if (_DefBuffRoundsRemaining == 0)
        {
            _Stats._TempDef = 0;
        }
    }

    // 민첩성 버프 처리
    if (_DexBuffRoundsRemaining > 0)
    {
        _DexBuffRoundsRemaining--;
        if (_DexBuffRoundsRemaining == 0)
        {
            _Stats._TempDex = 0;
        }
    }

    // 운 버프 처리
    if (_LukBuffRoundsRemaining > 0)
    {
        _LukBuffRoundsRemaining--;
        if (_LukBuffRoundsRemaining == 0)
        {
            _Stats._TempLuk = 0;
        }
    }

    // 치명타율 버프 처리
    if (_CriticalRateBuffRoundsRemaining > 0)
    {
        _CriticalRateBuffRoundsRemaining--;
        if (_CriticalRateBuffRoundsRemaining == 0)
        {
            _Stats._TempCriticalRate = 0.0f;
        }
    }

    // 어그로 고정 처리
    if (_IsAggroLocked && _AggroLockRoundsRemaining > 0)
    {
        _AggroLockRoundsRemaining--;
        if (_AggroLockRoundsRemaining == 0)
        {
            // 어그로 고정 해제 및 0으로 초기화
            UnlockAggro();
        }
        else
        {
            // 고정 유지
            _AggroValue = _LockedAggroValue;
        }
    }

    // 스킬 쿨타임 처리
    ProcessSkillCooldowns();
}

void Player::ResetBuffs()
{
    _Stats._TempAtk = 0;
    _Stats._TempDef = 0;
    _Stats._TempDex = 0;
    _Stats._TempLuk = 0;
    _Stats._TempCriticalRate = 0.0f;
    _AtkBuffRoundsRemaining = 0;
    _DefBuffRoundsRemaining = 0;
    _DexBuffRoundsRemaining = 0;
    _LukBuffRoundsRemaining = 0;
    _CriticalRateBuffRoundsRemaining = 0;

    // 어그로 고정 상태도 초기화
    _IsAggroLocked = false;
    _AggroLockRoundsRemaining = 0;
    _LockedAggroValue = 0;
}

// 버프 포함 총 스탯 조회 메서드들
int Player::GetTotalAtk() const
{
    return _Stats._Atk + _Stats._TempAtk;
}

int Player::GetTotalDef() const
{
    return _Stats._Def + _Stats._TempDef;
}

int Player::GetTotalDex() const
{
    return _Stats._Dex + _Stats._TempDex;
}

int Player::GetTotalLuk() const
{
    return _Stats._Luk + _Stats._TempLuk;
}

float Player::GetTotalCriticalRate() const
{
    return _Stats._CriticalRate + _Stats._TempCriticalRate;
}

// ===== 스킬 시스템 구현 =====

void Player::AddSkill(std::unique_ptr<ISkill> skill)
{
    if (skill)
    {
        _Skills.push_back(std::move(skill));
    }
}

SkillResult Player::UseSkill(int skillIndex, ICharacter* target)
{
    if (skillIndex < 0 || skillIndex >= static_cast<int>(_Skills.size()))
    {
        SkillResult result;
        result.SkillName = "";
        result.Value = 0;
        result.HitCount = 0;
        result.Success = false;
        result.Message = "잘못된 스킬 인덱스";
        return result;
    }

    ISkill* skill = _Skills[skillIndex].get();
    if (!skill)
    {
        SkillResult result;
        result.SkillName = "";
        result.Value = 0;
        result.HitCount = 0;
        result.Success = false;
        result.Message = "스킬이 없습니다";
        return result;
    }

    // 쿨타임 확인
    int cooldown = GetSkillCooldown(skill->GetName());
    if (cooldown > 0)
    {
        SkillResult result;
        result.SkillName = skill->GetName();
        result.Value = 0;
        result.HitCount = 0;
        result.Success = false;
        result.Message = "쿨타임 중입니다 (" + std::to_string(cooldown) + "턴 남음)";
        return result;
    }

    // MP 확인
    if (_Stats._CurrentMP < skill->GetMPCost())
    {
        SkillResult result;
        result.SkillName = skill->GetName();
        result.Value = 0;
        result.HitCount = 0;
        result.Success = false;
        result.Message = "MP가 부족합니다";
        return result;
    }

    // 스킬 사용 조건 확인
    if (!skill->CanActivate(this))
    {
        SkillResult result;
        result.SkillName = skill->GetName();
        result.Value = 0;
        result.HitCount = 0;
        result.Success = false;
        result.Message = skill->GetConditionDescription();
        return result;
    }

    // 스킬 사용 (효과 계산)
    SkillResult result = skill->CalculateEffect(this, target);

    if (result.Success)
    {
        // MP 소모
        ModifyMP(-skill->GetMPCost());
        TrackMPSpent(skill->GetMPCost());

        // 쿨타임 설정
        _SkillCooldowns[skill->GetName()] = skill->GetCooldown();
    }

    return result;
}

bool Player::CanUseSkill(int skillIndex) const
{
    if (skillIndex < 0 || skillIndex >= static_cast<int>(_Skills.size()))
    {
        return false;
    }

    const ISkill* skill = _Skills[skillIndex].get();
    if (!skill)
    {
        return false;
    }

    // 쿨타임 확인
    if (GetSkillCooldown(skill->GetName()) > 0)
    {
        return false;
    }

    // MP 확인
    if (_Stats._CurrentMP < skill->GetMPCost())
    {
        return false;
    }

    // 스킬 사용 조건 확인
    if (!skill->CanActivate(this))
    {
        return false;
    }

    return true;
}

int Player::SelectBestSkill(ICharacter* target) const
{
    // 기본 구현: 사용 가능한 첫 번째 스킬 선택
    for (int i = 0; i < static_cast<int>(_Skills.size()); ++i)
    {
        if (CanUseSkill(i))
        {
            return i;
        }
    }

    return -1;  // 사용 가능한 스킬 없음
}

void Player::ProcessSkillCooldowns()
{
    for (auto& pair : _SkillCooldowns)
    {
        if (pair.second > 0)
        {
            pair.second--;
        }
    }
}

int Player::GetSkillCooldown(const std::string& skillName) const
{
    auto it = _SkillCooldowns.find(skillName);
    if (it != _SkillCooldowns.end())
    {
        return it->second;
    }
    return 0;
}

// ===== 어그로 시스템 구현 =====

void Player::ModifyAggro(int amount)
{
    // 어그로가 고정되어 있으면 변경 불가
    if (_IsAggroLocked)
        return;

    _AggroValue += amount;

    // 0~100 범위 제한
    if (_AggroValue < 0)
    {
        _AggroValue = 0;
    }
    if (_AggroValue > 100)
    {
        _AggroValue = 100;
    }
}

void Player::ResetAggro()
{
    // 기본값 0 (전사는 자식 클래스에서 30으로 오버라이드)
    _AggroValue = 0;

    // 어그로 고정 상태 초기화
    _IsAggroLocked = false;
    _AggroLockRoundsRemaining = 0;
    _LockedAggroValue = 0;
}

void Player::LockAggro(int value, int rounds)
{
    _IsAggroLocked = true;
    _AggroLockRoundsRemaining = rounds;
    _LockedAggroValue = value;
    _AggroValue = value;  // 즉시 어그로 변경
}

void Player::UnlockAggro()
{
    _IsAggroLocked = false;
    _AggroLockRoundsRemaining = 0;
    _AggroValue = 0;  // 어그로 0으로 초기화
}

std::string Player::GetAggroMaxDialogue() const
{
    // 기본 대사 (각 직업 클래스에서 오버라이드)
    return _Name + ": 내가 상대다!";
}

// ===== 숙련도 시스템 구현 (FF2 스타일) =====

void Player::GainHPProficiency(int amount)
{
    _HPProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::GainMPProficiency(int amount)
{
    _MPProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::GainAtkProficiency(int amount)
{
    _AtkProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::GainDefProficiency(int amount)
{
    _DefProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::GainDexProficiency(int amount)
{
    _DexProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::GainMagicProficiency(int amount)
{
    _MagicProficiency += amount;
    CheckProficiencyGrowth();
}

void Player::ProcessBattleEndProficiency()
{
    PrintManager* pm = PrintManager::GetInstance();

    // ===== 1. HP 숙련도 (전사, 모든 직업) =====
    if (_DamageTakenThisBattle >= 50)
    {
        int hpGrowthChances = _DamageTakenThisBattle / 50;
        int totalHPGain = 0;

        for (int i = 0; i < hpGrowthChances; ++i)
        {
            int growth = 2 + (rand() % 4);
            _Stats._MaxHP += growth;
            totalHPGain += growth;
        }

        if (totalHPGain > 0)
        {
            pm->PrintLogLine(
                _Name + "의 체력이 단련되었습니다! (Max HP +" + std::to_string(totalHPGain) + ")",
                ELogImportance::DISPLAY
            );
        }
    }

    // ===== 2. MP/지력 숙련도 (마법사, 사제) =====
    if (_MPSpentThisBattle >= 50)
    {
        int mpPoints = _MPSpentThisBattle / 50;
        _MagicProficiency += mpPoints;

        int mpGain = mpPoints * 2;
        _Stats._MaxMP += mpGain;

        pm->PrintLogLine(
            _Name + "의 마력이 성장했습니다! (Max MP +" + std::to_string(mpGain) + ")",
            ELogImportance::DISPLAY
        );
    }

    // ===== 3. 민첩/치명타 숙련도 (궁수) =====
    if (_CriticalHitsThisBattle >= 5)
    {
        int critPoints = _CriticalHitsThisBattle / 5;

        _Stats._Dex += critPoints;

        float critRateGain = critPoints * 0.001f;
        _Stats._CriticalRate += critRateGain;

        pm->PrintLogLine(
            _Name + "의 민첩성이 향상되었습니다! (DEX +" + std::to_string(critPoints) +
            ", Crit Rate +" + std::to_string(static_cast<int>(critRateGain * 100)) + "%)",
            ELogImportance::DISPLAY
        );
    }

    // ===== 4. 회복 숙련도 (사제 전용) =====
    if (_HealingDoneThisBattle >= 1000)
    {
        int healPoints = _HealingDoneThisBattle / 1000;

        _MagicProficiency += (healPoints * 10);

        pm->PrintLogLine(
            _Name + "의 치유력이 성장했습니다! (회복 숙련도 +" + std::to_string(healPoints * 10) + ")",
            ELogImportance::DISPLAY
        );
    }

    ApplyProficiencyGrowth();

    // 전투 추적 초기화
    _DamageTakenThisBattle = 0;
    _MPSpentThisBattle = 0;
    _CriticalHitsThisBattle = 0;
    _HealingDoneThisBattle = 0;
}

void Player::TrackDamageTaken(int amount)
{
    _DamageTakenThisBattle += amount;
}

void Player::TrackMPSpent(int amount)
{
    _MPSpentThisBattle += amount;
}

void Player::TrackCriticalHit()
{
    _CriticalHitsThisBattle++;
}

void Player::TrackHealing(int amount)
{
    _HealingDoneThisBattle += amount;
}

void Player::CheckProficiencyGrowth()
{
    // 기본 Player 클래스에서는 빈 구현
    // 각 직업 클래스에서 오버라이드하여 세부 구현
    // 
    // 구현 예시:
    // - Warrior: HP 숙련도가 일정 수치 도달 시 추가 방어력 증가
    // - Mage: 마법 숙련도로 스킬 데미지 증가
    // - Archer: 민첩 숙련도로 회피율 증가
    // - Priest: 마법 숙련도(회복)로 힐링 스킬 강화
}
