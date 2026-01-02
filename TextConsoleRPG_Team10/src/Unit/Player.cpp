#include "../../include/Unit/Player.h"
#include <iostream>
#include "../../include/Manager/PrintManager.h"

Player::Player(const std::string& Name) : _Inventory(10)
{
    _Name = Name;
    // To-Do : csv 파일에서 플레이어 초기 스탯 불러오기
    _Level = 1;
    _MaxHP = 200;
    _CurrentHP = _MaxHP;
    _Atk = 30;
    _MaxExp = 100;
    _CurrentExp = 0;
    _Gold = 100;

    // 버프 초기화
    _TempAtkBonus = 0;
    _BuffRoundsRemaining = 0;
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
    if (!Target)
        return;

    // 버프 포함한 총 공격력로 공격
    int TotalDamage = _Atk + _TempAtkBonus;
    Target->TakeDamage(TotalDamage);
}

std::string Player::GetAttackNarration() const
{
    return _Name + "이(가) 용감하게 공격을 날립니다!";
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
    if (_Level >= 10)
    {
        // To-Do : 최대 레벨 도달 시 처리
    }
    else
    {
        _Level++;
        //_MaxHP += (_Level * 20);
        _CurrentHP = _MaxHP;
        _Atk += (_Level * 5);
        //_MaxExp += static_cast<int>(_MaxExp * 1.2f);

        // 로그 출력
        PrintManager::GetInstance()->PrintLogLine(_Name + "은(는) "+"LV" + std::to_string(_Level)+"이(가) 되었습니다!");
        PrintManager::GetInstance()->EndLine();
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

void Player::UseItem(const int SlotIndex)
{
    _Inventory.UseItem(SlotIndex, *this);
}

// 범용 스탯 수정 메서드들
void Player::ModifyHP(const int Amount)
{
    _CurrentHP += Amount;

    // 최대 HP 제한
    if (_CurrentHP > _MaxHP)
    {
        _CurrentHP = _MaxHP;
    }

    // 최소 0 제한
    if (_CurrentHP < 0)
    {
        _CurrentHP = 0;
    }
}

void Player::ModifyMaxHP(const int Amount)
{
    _MaxHP += Amount;

    // 최대 HP가 줄어들어 현재 HP가 초과하는 경우 조정
    if (_CurrentHP > _MaxHP)
    {
        _CurrentHP = _MaxHP;
    }
}

void Player::ModifyAtk(const int Amount)
{
    _Atk += Amount;

    // 공격력 최소값 제한 (0 이하로 내려가지 않도록)
    if (_Atk < 0)
    {
        _Atk = 0;
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
void Player::ApplyTempAtkBuff(const int Amount, const int Rounds)
{
    _TempAtkBonus += Amount;
    _BuffRoundsRemaining = Rounds;
}

void Player::ProcessRoundEnd()
{
    if (_BuffRoundsRemaining > 0)
    {
        _BuffRoundsRemaining--;
    
        // 버프가 끝났다면 효과 제거
        if (_BuffRoundsRemaining == 0)
        {
            _TempAtkBonus = 0;
        }
    }
}

void Player::ResetBuffs()
{
    _TempAtkBonus = 0;
    _BuffRoundsRemaining = 0;
}

int Player::GetTotalAtk() const
{
    return _Atk + _TempAtkBonus;
}
