#include "../../include/Unit/NormalMonster.h"
#include "../../include/Manager/BattleManager.h" // Player 클래스 선언 추가
#include <random>

// GameManager에 추가 후 삭제
static mt19937 gen(random_device{}());

NormalMonster::NormalMonster(const int PlayerLevel) 
{
    _Name = "Normal Monster";
    _Level = PlayerLevel;

    uniform_int_distribution<> HpDist(_Level * 20, _Level * 30);
    _MaxHP = HpDist(gen);
    _CurrentHP = _MaxHP;

    uniform_int_distribution<> AtkDist(_Level * 5, _Level * 10);
    _Atk = AtkDist(gen);
}

void NormalMonster::TakeDamage(int amount)
{
    // 데미지 받음
    _CurrentHP -= amount;
}

void NormalMonster::Attack(ICharacter* target)
{
    // 공격 연출 등 나중에 추가하면 될 듯
    target->TakeDamage(_Atk);
}

bool NormalMonster::IsDead()
{
    // Dead 여부 확인
    return _CurrentHP <= 0;
}

void NormalMonster::DropReward()
{
  // 리워드 드롭
    // 경험치 50, 골드 10~20, 30% 확률 아이템 드롭
    // 튜플로 전달
    // TODO: 리워드 계산
    //BattleManager::GetInstance()->CalculateReward(튜플);
}
