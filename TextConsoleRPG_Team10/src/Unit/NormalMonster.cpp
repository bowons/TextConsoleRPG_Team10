#include "../../include/Unit/NormalMonster.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include <random>
#include <tuple>
#include <memory>

using namespace std;

// GameManager에 추가 후 삭제
static mt19937 gen(random_device{}());

NormalMonster::NormalMonster(int PlayerLevel)
{
    _Name = "Normal Monster";
    _Level = PlayerLevel;

    uniform_int_distribution<> HpDist(_Level * 20, _Level * 30);
    _MaxHP = HpDist(gen);
    _CurrentHP = _MaxHP;

    uniform_int_distribution<> AtkDist(_Level * 5, _Level * 10);
    _Atk = AtkDist(gen);
}

void NormalMonster::TakeDamage(int Amount)
{
    // 데미지 받음
    _CurrentHP -= Amount;
    if (_CurrentHP < 0) 
    {
        _CurrentHP = 0;
    }
}

void NormalMonster::Attack(ICharacter* Target) const
{
    if (!Target) 
        return;

    // 공격 연출 등 나중에 추가하면 될 듯
    Target->TakeDamage(_Atk);
}

bool NormalMonster::IsDead() const
{
    // Dead 여부 확인
    return _CurrentHP <= 0;
}

tuple<int, int, unique_ptr<IItem>> NormalMonster::DropReward()
{
    // 경험치 50, 골드 10~20, 30% 확률 아이템 드롭
    unique_ptr<IItem> DropItem = nullptr;

    if (uniform_int_distribution<>(0, 9)(gen) < 3) // 30% 확률로 아이템 드롭
    {
        if (uniform_int_distribution<>(0, 1)(gen)) // 50% HealPotion, 50% AttackUp
        {
            // Clone으로 새 HealPotion 인스턴스 생성
            DropItem = HealPotion().Clone();
        }
        else
        {
            // Clone으로 새 AttackUp 인스턴스 생성
            DropItem = AttackUp().Clone();
        }
    }
    
    return {50, uniform_int_distribution<>(10, 20)(gen), move(DropItem)};
}

std::string NormalMonster::GetAttackNarration() const
{
    return _Name + "가 사납게 공격을 내지릅니다!";
}