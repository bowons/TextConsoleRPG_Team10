#include "../../include/Unit/EliteMonster.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Item/IItem.h"
#include "../../include/Unit/IMonster.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Item/FairyEssence.h"
#include "../../include/Item/ShieldDefense.h"
#include "../../include/Item/FocusRune.h"
#include "../../include/Item/LuckyCharm.h"
#include "../../include/Item/TitanAwakening.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Skill/MonsterSkills.h"
#include "../../include/Skill/ISkill.h"
#include <random>
#include <tuple>
#include <memory>
#include <vector>

EliteMonster::EliteMonster(const MonsterSpawnData& Data)
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

    // ===== CSV에서 공격명 로드 =====
    _AttackName = Data.attack_name;

    // ===== 임시 스탯은 기본 0 =====
    _Stats._TempAtk = 0;
    _Stats._TempDef = 0;
    _Stats._TempDex = 0;
    _Stats._TempLuk = 0;
    _Stats._TempCriticalRate = 0.0f;

    // ===== Elite 전용 초기화 =====
    _TurnCounter = 0;
    InitializeSkills();
}

void EliteMonster::InitializeSkills()
{
    // Elite 강공격 스킬 추가
    _Skills.push_back(std::make_unique<ElitePowerStrike>());
}

int EliteMonster::TakeDamage(ICharacter* Target, int Amount)
{
    // 데미지 받음
    // 회피율 = 5% + (피해자_DEX − 공격자_DEX) × 1.5%
    int Evasion = 5 + (Target->GetDex() - this->GetDex()) * 15 / 10;
    if (Evasion > 95) Evasion = 95; // 최대 회피율 95%
    if (std::uniform_int_distribution<>(1, 100)(gen) <= Evasion)
    {
        // 회피 성공
        Amount = 0;
        return Amount;
    }
    _Stats._CurrentHP -= Amount;
    if (_Stats._CurrentHP < 0)
    {
        _Stats._CurrentHP = 0;
    }
    return Amount;
}

std::tuple<std::string, int> EliteMonster::Attack(ICharacter* Target) const
{
    if (!Target)
        return { "", 0 };

    // 턴 카운터 증가 (공격할 때마다)
    _TurnCounter++;

    // 3턴마다 스킬 사용
    if (_TurnCounter % 3 == 0 && !_Skills.empty())
    {
        // Elite 강공격 스킬 사용 (첫 번째 스킬)
        // 스킬 데미지 계산 (기본 공격력 × 1.8배)
        int skillDamage = static_cast<int>(_Stats._Atk * 1.8f);
        return { "강력한 일격", skillDamage };
    }

    // ===== 치명타 판정 (LUK 반영) =====
    // 치명타율 = 기본 치명타율 + (총 LUK * 0.1%)
    int totalLuk = _Stats._Luk + _Stats._TempLuk;
    float lukBonus = totalLuk * 0.001f;  // LUK 1당 0.1% (0.001)
    float totalCritRate = _Stats._CriticalRate + _Stats._TempCriticalRate + lukBonus;

    // 확률 계산 (1~100 사이)
    int critRoll = std::uniform_int_distribution<>(1, 100)(gen);
    float critThreshold = totalCritRate * 100.0f;

    if (critRoll <= static_cast<int>(critThreshold))
    {
        // ===== 치명타 발동! (데미지 2배) =====
        int critDamage = _Stats._Atk * 2;
        return { _AttackName + " 치명타!", critDamage };
    }

    // ===== 일반 공격 - CSV에서 로드한 공격명 사용 =====
    return { _AttackName, _Stats._Atk };
}

bool EliteMonster::IsDead() const
{
    return _Stats._CurrentHP <= 0;
}

std::tuple<int, int, std::unique_ptr<IItem>> EliteMonster::DropReward()
{
    std::unique_ptr<IItem> DropItem = nullptr;

    // DataManager에서 아이템 목록 로드
    DataManager* dm = DataManager::GetInstance();
    std::vector<ItemData> items = dm->LoadItemData("Items.csv");

    if (!items.empty())
    {
        // Elite는 더 높은 확률로 드롭 (각 아이템 확률 × 1.5배)
        std::vector<std::pair<ItemData, float>> dropPool;

        for (const auto& item : items)
        {
            if (item.MonsterDropRate > 0.0f)
            {
                // Elite는 1.5배 확률 (최대 1.0으로 제한)
                float eliteDropRate = std::min(item.MonsterDropRate * 1.5f, 1.0f);
                dropPool.push_back({ item, eliteDropRate });
            }
        }

        if (!dropPool.empty())
        {
            // 0~1 사이의 랜덤 값 생성
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            float roll = dist(gen);

            // 누적 확률로 아이템 선택
            float cumulative = 0.0f;
            for (const auto& [itemData, dropRate] : dropPool)
            {
                cumulative += dropRate;
                if (roll <= cumulative)
                {
                    // ItemID로 아이템 인스턴스 생성
                    if (itemData.ItemID == "HealPotion")
                    {
                        DropItem = HealPotion().Clone();
                    }
                    else if (itemData.ItemID == "FairyEssence")
                    {
                        DropItem = FairyEssence().Clone();
                    }
                    else if (itemData.ItemID == "AttackUp")
                    {
                        DropItem = AttackUp().Clone();
                    }
                    else if (itemData.ItemID == "ShieldDefense")
                    {
                        DropItem = ShieldDefense().Clone();
                    }
                    else if (itemData.ItemID == "FocusRune")
                    {
                        DropItem = FocusRune().Clone();
                    }
                    else if (itemData.ItemID == "LuckyCharm")
                    {
                        DropItem = LuckyCharm().Clone();
                    }
                    else if (itemData.ItemID == "TitanAwakening")
                    {
                        DropItem = TitanAwakening().Clone();
                    }
                    break;
                }
            }
        }
    }

    return { _ExpReward, _GoldReward, std::move(DropItem) };
}

std::string EliteMonster::GetAttackNarration() const
{
    // 3턴마다 특별 연출
    if (_TurnCounter % 3 == 0)
    {
        return "💥 " + _Name + "이(가) 강력한 일격을 준비한다! 💥";
    }
    return _Name + "이(가) 사납게 공격을 내지릅니다!";
}
