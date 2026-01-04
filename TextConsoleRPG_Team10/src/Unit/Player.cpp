#include "../../include/Unit/Player.h"
#include <iostream>
#include "../../include/Manager/PrintManager.h"

Player::Player(const std::string& Name, bool enableInventory)
{
    _Name = Name;
    // To-Do : csv 파일에서 플레이어 초기 스탯 불러오기
    _Level = 1;
    _Stats._MaxHP = 200;
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._Atk = 30;
    _MaxExp = 100;
    _CurrentExp = 0;
    _Gold = 100;

    // 인벤토리 활성화 여부에 따라 생성
    if (enableInventory)
    {
        _Inventory = std::make_unique<Inventory>(10);  // 10슬롯 인벤토리
    }
    // else: _Inventory는 nullptr (동료 캐릭터)

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
}

void Player::TakeDamage(const int Amount)
{
  _Stats._CurrentHP -= Amount;
  if (_Stats._CurrentHP < 0)
    {
	  _Stats._CurrentHP = 0;
    }
}

void Player::Attack(ICharacter* Target) const
{
    if (!Target)
        return;

    // 버프 포함한 총 공격력로 공격
    int TotalDamage = _Stats._Atk + _Stats._TempAtk;
    Target->TakeDamage(TotalDamage);
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
        _CurrentExp -= _MaxExp;
        ProcessLevelUp();
    }
}

void Player::ProcessLevelUp()
{
    if (_Level >= 10)
    {
        // To-Do : 최대 레벨 도달 시 처리
        return;
    }

    _Level++;
    //_MaxHP += (_Level * 20);
    _Stats._CurrentHP = _Stats._MaxHP;
    _Stats._Atk += (_Level * 5);
    //_MaxExp += static_cast<int>(_MaxExp * 1.2f);

    // PrintManager 제거 - Scene에서 표시
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
    // 인벤토리 확인 (한 번에 처리)
    Inventory* inventory = nullptr;
    if (!TryGetInventory(inventory))
    {
        return false;  // 인벤토리 없음
 }
    
    // 인벤토리에서 아이템 사용
    return inventory->UseItem(SlotIndex, *this);
}

// 범용 스탯 수정 메서드들
void Player::ModifyHP(const int Amount)
{
	_Stats._CurrentHP += Amount;

    // 최대 HP 제한
    if (_Stats._CurrentHP > _Stats._MaxHP)
    {
		_Stats._CurrentHP = _Stats._MaxHP;
    }

    // 최소 0 제한
    if (_Stats._CurrentHP < 0)
    {
		_Stats._CurrentHP = 0;
    }
}

void Player::ModifyMaxHP(const int Amount)
{
	_Stats._MaxHP += Amount;

    // 최대 HP가 줄어들어 현재 HP가 초과하는 경우 조정
	if (_Stats._CurrentHP > _Stats._MaxHP)
    {
		_Stats._CurrentHP = _Stats._MaxHP;
    }
}

void Player::ModifyMP(const int Amount)
{
	_Stats._CurrentMP += Amount;
	// 최대 MP 제한
	if (_Stats._CurrentMP > _Stats._MaxMP)
	{
		_Stats._CurrentMP = _Stats._MaxMP;
	}
	// 최소 0 제한
	if (_Stats._CurrentMP < 0)
	{
		_Stats._CurrentMP = 0;
	}
}

void Player::ModifyMaxMP(const int Amount)
{
	_Stats._MaxMP += Amount;
	// 최대 MP가 줄어들어 현재 MP가 초과하는 경우 조정
	if (_Stats._CurrentMP > _Stats._MaxMP)
	{
		_Stats._CurrentMP = _Stats._MaxMP;
	}
}

void Player::ModifyAtk(const int Amount)
{
	_Stats._Atk += Amount;

    // 공격력 최소값 제한 (0 이하로 내려가지 않도록)
    if (_Stats._Atk < 0)
    {
		_Stats._Atk = 0;
    }
}

void Player::ModifyDef(const int Amount)
{
	_Stats._Def += Amount;
	// 방어력 최소값 제한
	if (_Stats._Def < 0)
	{
		_Stats._Def = 0;
	}
}

void Player::ModifyDex(const int Amount)
{
	_Stats._Dex += Amount;
	// 민첩성 최소값 제한
	if (_Stats._Dex < 0)
	{
		_Stats._Dex = 0;
	}
}

void Player::ModifyLuk(const int Amount)
{
	_Stats._Luk += Amount;
	// 운 최소값 제한
	if (_Stats._Luk < 0)
	{
		_Stats._Luk = 0;
	}
}

void Player::ModifyCriticalRate(const float Amount)
{
	_Stats._CriticalRate += Amount;
	// 치명타율 최소값 제한
	if (_Stats._CriticalRate < 0.0f)
	{
		_Stats._CriticalRate = 0.0f;
	}
}

void Player::ModifyGold(const int Amount)
{
    _Gold += Amount;

    // 골드 최소값 제한
    if (_Gold < 0)
    {
        _Gold = 0;
    }
}

// 버프 관리 메서드들
void Player::ApplyTempAtkBuff(const int Amount, const int Rounds) {
  _Stats._TempAtk += Amount;
  _AtkBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempDefBuff(const int Amount, const int Rounds) {
  _Stats._TempDef += Amount;
  _DefBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempDexBuff(const int Amount, const int Rounds) {
  _Stats._TempDex += Amount;
  _DexBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempLukBuff(const int Amount, const int Rounds) {
  _Stats._TempLuk += Amount;
  _LukBuffRoundsRemaining = Rounds;
}

void Player::ApplyTempCriticalRateBuff(const float Amount, const int Rounds) {
  _Stats._TempCriticalRate += Amount;
  _CriticalRateBuffRoundsRemaining = Rounds;
}

void Player::ProcessRoundEnd() {
  // 공격력 버프 처리
  if (_AtkBuffRoundsRemaining > 0) {
    _AtkBuffRoundsRemaining--;
    if (_AtkBuffRoundsRemaining == 0) {
      _Stats._TempAtk = 0;
    }
  }

  // 방어력 버프 처리
  if (_DefBuffRoundsRemaining > 0) {
    _DefBuffRoundsRemaining--;
    if (_DefBuffRoundsRemaining == 0) {
      _Stats._TempDef = 0;
    }
  }

  // 민첩성 버프 처리
  if (_DexBuffRoundsRemaining > 0) {
    _DexBuffRoundsRemaining--;
    if (_DexBuffRoundsRemaining == 0) {
      _Stats._TempDex = 0;
    }
  }

  // 운 버프 처리
  if (_LukBuffRoundsRemaining > 0) {
    _LukBuffRoundsRemaining--;
    if (_LukBuffRoundsRemaining == 0) {
      _Stats._TempLuk = 0;
    }
  }

  // 치명타율 버프 처리
  if (_CriticalRateBuffRoundsRemaining > 0) {
    _CriticalRateBuffRoundsRemaining--;
    if (_CriticalRateBuffRoundsRemaining == 0) {
      _Stats._TempCriticalRate = 0.0f;
    }
  }
}

void Player::ResetBuffs() {
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
}

// 버프 포함 총 스탯 조회 메서드들
int Player::GetTotalAtk() const { return _Stats._Atk + _Stats._TempAtk; }

int Player::GetTotalDef() const { return _Stats._Def + _Stats._TempDef; }

int Player::GetTotalDex() const { return _Stats._Dex + _Stats._TempDex; }

int Player::GetTotalLuk() const { return _Stats._Luk + _Stats._TempLuk; }

float Player::GetTotalCriticalRate() const {
  return _Stats._CriticalRate + _Stats._TempCriticalRate;
}
