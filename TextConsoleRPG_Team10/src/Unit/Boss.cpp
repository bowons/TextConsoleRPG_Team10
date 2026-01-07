#include "../../include/Unit/Boss.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Item/IItem.h"
#include "../../include/Unit/IMonster.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/SoundPlayer.h"
#include "../../include/Skill/MonsterSkills.h"
#include "../../include/Skill/ISkill.h"
#include <random>
#include <tuple>
#include <memory>

Boss::Boss(const MonsterSpawnData& Data)
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

    // ===== Boss 전용 초기화 =====
    _TurnCounter = 0;
    _IsPhase2 = false;
    InitializeSkills();
}

void Boss::InitializeSkills()
{
    // 페이즈 1: 단일 강공격
    _Skills.push_back(std::make_unique<BossPhase1Attack>());

    // 페이즈 2: 광역 공격
    _Skills.push_back(std::make_unique<BossPhase2AOE>());

    // 페이즈 2: 디버프
    _Skills.push_back(std::make_unique<BossDebuff>());
}

void Boss::CheckPhase() const
{
    // HP 50% 미만이면 페이즈 2
    float hpRatio = static_cast<float>(_Stats._CurrentHP) / static_cast<float>(_Stats._MaxHP);
    _IsPhase2 = (hpRatio < 0.5f);
}

int Boss::TakeDamage(ICharacter* Target, int Amount)
{
    // 데미지 받음
    // 회피율 = 5% + (피해자_DEX − 공격자_DEX) × 1.5%
    int Evasion = 5 + (Target->GetDex() - this->GetDex()) * 15 / 10;
    if (Evasion > 95) Evasion = 95; // 최대 회피율 95%
    if (std::uniform_int_distribution<>(1, 100)(gen) <= Evasion)
    {
        // 회피 성공
        Amount = 0;
        return -1;
    }

    _Stats._CurrentHP -= Amount;
    if (_Stats._CurrentHP < 0)
    {
        _Stats._CurrentHP = 0;
        SoundPlayer::GetInstance()->PlayMonsterSFX(GetName(), "_Dead");
    }

    // 페이즈 체크
    CheckPhase();

    return Amount;
}

std::tuple<std::string, int> Boss::Attack(ICharacter* Target) const
{
    if (!Target)
        return { "", 0 };

    // 페이즈 체크
    CheckPhase();

    // 턴 카운터 증가
    _TurnCounter++;

    // ===== 페이즈 2 (HP 50% 미만) =====
    if (_IsPhase2)
    {
        // 3턴마다 광역 공격
        if (_TurnCounter % 3 == 0)
        {
            int aoeDamage = static_cast<int>(_Stats._Atk * 1.2f);
            SoundPlayer::GetInstance()->PlayMonsterSFX(GetName(), "_Attack1");
            return { "어둠의 폭풍", aoeDamage };
        }

        // 4턴마다 디버프
        if (_TurnCounter % 4 == 0)
        {
            // 디버프는 데미지 0, BattleManager에서 처리
            SoundPlayer::GetInstance()->PlayMonsterSFX(GetName(), "_Debuff");
            return { "공포의 속삭임", 0 };
        }

        // ===== 치명타 판정 (LUK 반영, 강화된 일반 공격) =====
        int totalLuk = _Stats._Luk + _Stats._TempLuk;
        float lukBonus = totalLuk * 0.001f;  // LUK 1당 0.1%
        float totalCritRate = _Stats._CriticalRate + _Stats._TempCriticalRate + lukBonus;

        int critRoll = std::uniform_int_distribution<>(1, 100)(gen);
        float critThreshold = totalCritRate * 100.0f;

        if (critRoll <= static_cast<int>(critThreshold))
        {
            // 치명타 발동! (강화된 공격 × 2배)
            int critDamage = static_cast<int>(_Stats._Atk * 1.3f * 2);
            SoundPlayer::GetInstance()->PlaySFX("Golem_Atack");
            return { "강화된 " + _AttackName + " 치명타!", critDamage };
        }

        // 그 외엔 강화된 일반 공격 (×1.3배)
        int enhancedDamage = static_cast<int>(_Stats._Atk * 1.3f);
        SoundPlayer::GetInstance()->PlaySFX("Golem_Atack");
        return { "강화된 " + _AttackName, enhancedDamage };
    }

    // ===== 페이즈 1 (HP 50% 이상) =====
    // 2턴마다 에테르 충격파
    if (_TurnCounter % 2 == 0)
    {
        int skillDamage = static_cast<int>(_Stats._Atk * 1.5f);
        SoundPlayer::GetInstance()->PlayMonsterSFX(GetName(), "_Attack2");
        return { "에테르 충격파", skillDamage };
    }

    // ===== 치명타 판정 (LUK 반영, 일반 공격) =====
    int totalLuk = _Stats._Luk + _Stats._TempLuk;
    float lukBonus = totalLuk * 0.001f;  // LUK 1당 0.1%
    float totalCritRate = _Stats._CriticalRate + _Stats._TempCriticalRate + lukBonus;

    int critRoll = std::uniform_int_distribution<>(1, 100)(gen);
    float critThreshold = totalCritRate * 100.0f;

    if (critRoll <= static_cast<int>(critThreshold))
    {
        // ===== 치명타 발동! (데미지 2배) =====
        int critDamage = _Stats._Atk * 2;
        SoundPlayer::GetInstance()->PlaySFX("Golem_Atack");
        return { _AttackName + " 치명타!", critDamage };
    }

    // ===== 일반 공격 - CSV에서 로드한 공격명 사용 =====
    return { _AttackName, _Stats._Atk };
}

bool Boss::IsDead() const
{
    return _Stats._CurrentHP <= 0;
}

std::tuple<int, int, std::unique_ptr<IItem>> Boss::DropReward()
{
    // 게임 엔딩인데 아이템 드롭 없음 (경험치와 골드만)
    std::unique_ptr<IItem> DropItem = nullptr;

    return { _ExpReward, _GoldReward, std::move(DropItem) };
}

std::string Boss::GetAttackNarration() const
{
    if (_IsPhase2)
    {
        if (_TurnCounter % 3 == 0)
        {
            return "🌪️ 에테르노가 어둠의 힘을 모은다! 파티 전체 위험! 🌪️";
        }
        if (_TurnCounter % 4 == 0)
        {
            return "😱 에테르노의 공포스러운 속삭임이 울려퍼진다! 😱";
        }
        return "⚡ 에테르노가 분노하며 강화된 공격을 퍼붓는다! ⚡";
    }

    if (_TurnCounter % 2 == 0)
    {
        return "💫 에테르노가 에테르 충격파를 발산한다! 💫";
    }

    return _Name + "이(가) 위압적인 공격을 시도합니다!";
}