#include "../../include/Unit/Player.h"
#include <iostream>

Player::Player(const string& name) 
{
  // To-Do : csv 파일에서 플레이어 초기 스탯 불러오기
  _Name = name;
  _Level = 1;
  _MaxHP = 200;
  _CurrentHP = _MaxHP;
  _Atk = 30;
  _MaxExp = 550;
  _CurrentExp = 0;
  _Gold = 100;
}

void Player::TakeDamage(const int amount) 
{
  _CurrentHP -= amount;
  if (_CurrentHP < 0) 
  {
    _CurrentHP = 0;
  }
  cout << _Name << "이(가) " << amount << "의 피해를 입었습니다.\n"
       << "(남은 체력: " << _CurrentHP << "/" << _MaxHP << ")\n";
}

void Player::Attack(ICharacter* target) 
{
  if (target == nullptr) 
  {
    cout << "공격 대상이 없습니다.\n";
    return;
  }
  cout << _Name << "이(가) " << target->_Name << "을(를) 공격합니다!\n";
  target->TakeDamage(_Atk);
}

bool Player::IsDead() 
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

void Player::GainExp(int amount) 
{
  _CurrentExp += amount;
  cout << amount << "의 경험치를 획득했습니다! (현재 경험치: " << _CurrentExp
       << "/" << _MaxExp << ")\n";
  CheckLevelUp();
}

void Player::GainGold(int amount) 
{
  _Gold += amount;
  cout << amount << "골드를 획득했습니다! (보유 골드: " << _Gold << ")\n";
}

void Player::UseItem(int slotIndex) 
{ 
  _Inventory.UseItem(slotIndex, *this); 
}
