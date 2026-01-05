#include "../../include/Unit/NormalMonster.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Item/IItem.h"
#include "../../include/Unit/IMonster.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Manager/GameManager.h"
#include <random>
#include <tuple>
#include <memory>

// GameManager에 추가 후 삭제
//static std::mt19937 gen(std::random_device{}());

//NormalMonster::NormalMonster(int PlayerLevel, std::string Stage, std::string Name)
//{
//    Name = Name;
//    _Level = PlayerLevel;
//    _Stage = Stage;
//
//    std::uniform_int_distribution<> HpDist(_Level * 20, _Level * 30);
//    _Stats._MaxHP = HpDist(gen);
//    _Stats._CurrentHP = _Stats._MaxHP;
//
//    std::uniform_int_distribution<> AtkDist(_Level * 5, _Level * 10);
//    _Stats._Atk = AtkDist(gen);
//}

NormalMonster::NormalMonster(const MonsterSpawnData& Data)
{
    _Name = Data.MonsterName;
    _Floor = Data.floor;

    // ===== 기본 스탯 =====
    _Stats._MaxHP = Data.hp;
    _Stats._CurrentHP = Data.hp;

    _Stats._MaxMP = Data.mp;
    _Stats._CurrentMP = Data.mp;

    _Stats._Atk = Data.atk;
    _Stats._Def = Data.def;
    _Stats._Dex = Data.dex;
    _Stats._Luk = Data.luk;

    _Stats._CriticalRate = static_cast<float>(Data.crit_rate);
    _ExpReward = Data.exp;
    _GoldReward = Data.gold;

    // ===== 임시 스탯은 기본 0 =====
    _Stats._TempAtk = 0;
    _Stats._TempDef = 0;
    _Stats._TempDex = 0;
    _Stats._TempLuk = 0;
    _Stats._TempCriticalRate = 0.0f;
}


void NormalMonster::TakeDamage(int Amount)
{
    // 데미지 받음
    _Stats._CurrentHP -= Amount;
    if (_Stats._CurrentHP < 0) 
    {
		_Stats._CurrentHP = 0;
    }
}

void NormalMonster::Attack(ICharacter* Target) const
{
    if (!Target) 
        return;

    // 공격 연출 등 나중에 추가하면 될 듯
    Target->TakeDamage(_Stats._Atk);
}

bool NormalMonster::IsDead() const
{
    // Dead 여부 확인
	return _Stats._CurrentHP <= 0;
}

std::tuple<int, int, std::unique_ptr<IItem>> NormalMonster::DropReward()
{
    // 경험치 50, 골드 10~20, 30% 확률 아이템 드롭
    std::unique_ptr<IItem> DropItem = nullptr;

    if (std::uniform_int_distribution<>(0, 9)(gen) < 3) // 30% 확률로 아이템 드롭
    {
        if (std::uniform_int_distribution<>(0, 1)(gen)) // 50% HealPotion, 50% AttackUp
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
    return { _ExpReward, _GoldReward, std::move(DropItem) };
}

std::string NormalMonster::GetAttackNarration() const
{
    return _Name + "이(가) 사납게 공격을 내지릅니다!";
}