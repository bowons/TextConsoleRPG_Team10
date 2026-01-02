#pragma once
#include "ICharacter.h"
#include "../Item/Inventory.h"
#include <string>


class Player : public ICharacter
{
private:
    int _CurrentExp;
    int _MaxExp;
    int _Gold;
    Inventory _Inventory;
    
    // 임시 버프 관리
    int _TempAtkBonus;
    int _BuffRoundsRemaining;

public:
    Player(const std::string& Name);
    void TakeDamage(const int Amount) override;
    void Attack(ICharacter* Target) const override;
    bool IsDead() const override;
    
    void CheckLevelUp();
    void ProcessLevelUp();
    void GainExp(const int Amount);
    void GainGold(const int Amount);
    void UseItem(const int SlotIndex);

    inline int GetGold() const { return _Gold; }
    inline int GetMaxGold() const { return _MaxExp; }
    inline int GetExp() const { return _CurrentExp; }
    inline int GetMaxExp() const { return _MaxExp; }

	Inventory& GetInventory() { return _Inventory; }

    // 범용 스탯 수정 메서드 - 아이템이나 버프/디버프 등에서 사용
    void ModifyHP(int Amount);   // HP 증감 (최대치 제한)
    void ModifyMaxHP(int Amount);       // 최대 HP 증감
    void ModifyAtk(int Amount);  // 공격력 증감
    void ModifyGold(int Amount);     // 골드 증감
    
    // 버프 관리 메서드
    void ApplyTempAtkBuff(int Amount, int Rounds);
    void ProcessRoundEnd();
    void ResetBuffs();
    int GetTotalAtk() const;

    // 플레이어 전용 공격 연출
    std::string GetAttackNarration() const override;
};
