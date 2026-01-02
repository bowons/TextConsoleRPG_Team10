#include "../../include/Unit/Boss.h"
#include "../../include/Item/IItem.h"
#include "../../include/Unit/IMonster.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include <random>
#include <tuple>
#include <memory>
#include <iostream>

using namespace std;

// GameManager에 추가 후 삭제
static mt19937 gen(random_device{}());

Boss::Boss(int PlayerLevel)
{
    _Name = "에테르노";
    _Level = PlayerLevel;
    _Stage = "공허의 중심";

    uniform_int_distribution<> HpDist(_Level * 20, _Level * 30);
    _MaxHP = static_cast<int>(HpDist(gen) * 1.5); // 보스는 체력 1.5배
    _CurrentHP = _MaxHP;

    uniform_int_distribution<> AtkDist(_Level * 5, _Level * 10);
    _Atk = static_cast<int>(AtkDist(gen) * 1.5); // 보스는 공격력 1.5배
}

void Boss::TakeDamage(int Amount)
{
    // 데미지 받음
    _CurrentHP -= Amount;
    // 데미지 받는 연출 추가
    if (_CurrentHP < _MaxHP * 3 / 10)
    {
        std::cout << "test 30프로 남음 연출" << std::endl;
    }
    else if( _CurrentHP < _MaxHP * 6 / 10)
    {
        std::cout << "test 60프로 남음 연출" << std::endl;
    }
}

void Boss::Attack(ICharacter* Target) const
{
    // 공격 연출 등 나중에 추가하면 될 듯
    Target->TakeDamage(_Atk);
}

bool Boss::IsDead() const
{
    // Dead 여부 확인
    return _CurrentHP <= 0;
}

tuple<int, int, unique_ptr<IItem>> Boss::DropReward()
{
    // 게임 엔딩인데 다 뿌리자!
    unique_ptr<IItem> DropItem = nullptr;

    tuple<int, int, unique_ptr<IItem>> Reward(500, 50000, move(DropItem));
    return Reward;
}