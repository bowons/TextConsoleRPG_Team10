#include "../../include/Unit/Player.h"
#include <iostream>

Player::Player(const string& Name) 
{
  // To-Do : csv 파일에서 플레이어 초기 스탯 불러오기
  _Name = Name;
  _Level = 1;
  _MaxHP = 200;
  _CurrentHP = _MaxHP;
  _BaseAtk = 30;
  _CurrentAtk = _BaseAtk;
  _MaxExp = 550;
  _CurrentExp = 0;
  _Gold = 100;
}

void Player::TakeDamage(const int Amount) 
{
  _CurrentHP -= Amount;
  if (_CurrentHP < 0) 
  {
    _CurrentHP = 0;
  }
}

void Player::Attack(ICharacter* Target) const 
{
  if (Target == nullptr) 
  {
    return;
  }
}

bool Player::IsDead() const
{ 
	return _CurrentHP <= 0; 
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
  if (_Level > 10) 
  {
    // To-Do : 최대 레벨 도달 시 처리
  } 
  else 
  {
    _Level++;
    _MaxHP += (_Level * 20);
    _CurrentHP = _MaxHP;
    _BaseAtk += (_Level * 5);
    _CurrentAtk = _BaseAtk;
    _MaxExp += static_cast<int>(_MaxExp * 1.2f);
  }
}

void Player::GainExp(const int Amount) 
{
  _CurrentExp += Amount;
  CheckLevelUp();
}

void Player::GainGold(const int Amount) 
{
  _Gold += Amount;
}

void Player::UseItem(const int SlotIndex) 
{ 
  _Inventory.UseItem(SlotIndex, *this); 
}

void Player::AddAttack(const int Amount) 
{ 
  _CurrentAtk += Amount;
}

void Player::ResetAttack() 
{ 
  _CurrentAtk = _BaseAtk; 
}

void Player::Heal(const int Amount)
{ 
	if (_CurrentHP > _MaxHP || _CurrentHP + Amount > _MaxHP) 
	{
		_CurrentHP = _MaxHP; 
	} else 
	{
		_CurrentHP += Amount;
    }
}
