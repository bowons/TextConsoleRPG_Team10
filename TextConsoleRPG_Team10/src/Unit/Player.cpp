#include "../../include/Unit/Player.h"
#include <iostream>

Player::Player(const string& Name) 
{
  // To-Do : csv 파일에서 플레이어 초기 스탯 불러오기
  _Name = Name;
  _Level = 1;
  _MaxHP = 200;
  _CurrentHP = _MaxHP;
  _Atk = 30;
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
  cout << _Name << "이(가) " << Amount << "의 피해를 입었습니다.\n"
       << "(남은 체력: " << _CurrentHP << "/" << _MaxHP << ")\n";
}

void Player::Attack(ICharacter* Target) const 
{
  if (Target == nullptr) 
  {
    cout << "공격 대상이 없습니다.\n";
    return;
  }
  cout << _Name << "이(가) " << Target->_Name << "을(를) 공격합니다!\n";
  Target->TakeDamage(_Atk);
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
    cout << "최고 레벨에 도달했습니다!\n";
  } 
  else 
  {
    _Level++;
    _MaxHP += (_Level * 20);
    _CurrentHP = _MaxHP;
    _Atk += (_Level * 5);
    _MaxExp += static_cast<int>(_MaxExp * 1.2f);

    cout << "\n=== LEVEL UP! ===\n";
    cout << "레벨 " << _Level << " 달성!\n";
    cout << "최대 체력 증가: " << _MaximumHealthPoint << "\n";
    cout << "공격력 증가: " << _AttackPower << "\n";
    cout << "체력이 모두 회복되었습니다.\n";
  }
}

void Player::GainExp(const int Amount) 
{
  _CurrentExp += Amount;
  cout << Amount << "의 경험치를 획득했습니다! (현재 경험치: " << _CurrentExp
       << "/" << _MaxExp << ")\n";
  CheckLevelUp();
}

void Player::GainGold(const int Amount) 
{
  _Gold += Amount;
  cout << Amount << "골드를 획득했습니다! (보유 골드: " << _Gold << ")\n";
}

void Player::UseItem(const int SlotIndex) 
{ 
  _Inventory.UseItem(SlotIndex, *this); 
}
