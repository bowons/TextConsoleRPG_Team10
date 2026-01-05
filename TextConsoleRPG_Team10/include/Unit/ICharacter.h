#pragma once
#include <string>

// ===== 모든 캐릭터 공통 스탯 구조체 =====
struct CharacterStats {
  int _MaxHP = 0;      // 최대 체력
  int _CurrentHP = 0;  // 현재 체력
  int _MaxMP = 0;      // 최대 마나
  int _CurrentMP = 0;  // 현재 마나

  int _Atk = 0;  // 공격력
  int _Def = 0;  // 방어력
  int _Dex = 0;  // 민첩성 (공격 우선 순위에 영향)
  int _Luk = 0;  // 운 (치명타율에 영향)

  float _CriticalRate = 0.05f;  // 치명타 확률 (기본 5%)

  // ===== 임시 버프/디버프 =====
  int _TempAtk = 0;                // 임시 공격력 보정
  int _TempDef = 0;                // 임시 방어력 보정
  int _TempDex = 0;                // 임시 민첩성 보정
  int _TempLuk = 0;                // 임시 운 보정
  float _TempCriticalRate = 0.0f;  // 임시 치명타율 보정

  CharacterStats() = default;
};

class ICharacter
{
protected:
    std::string _Name;
    int _Level;
    CharacterStats _Stats;

public:
    virtual ~ICharacter() {}

    virtual void TakeDamage(const int Amount) = 0;
    virtual void Attack(ICharacter* Target) const = 0;
    virtual bool IsDead() const = 0;

    inline const std::string& GetName() const { return _Name; }
    inline int GetLevel() const { return _Level; }
    inline int GetCurrentHP() const { return _Stats._CurrentHP; }
    inline int GetMaxHP() const { return _Stats._MaxHP; }
    inline int GetCurrentMP() const { return _Stats._CurrentMP; }
    inline int GetMaxMP() const { return _Stats._MaxMP; }
    inline int GetAtk() const { return _Stats._Atk; }
    inline int GetDef() const { return _Stats._Def; }
    inline int GetDex() const { return _Stats._Dex; }
    inline int GetLuk() const { return _Stats._Luk; }
    inline float GetCriticalRate() const { return _Stats._CriticalRate; }

    // 공격 연출 문자열 반환 (기본값)
    virtual std::string GetAttackNarration() const { return _Name + "이(가) 공격을 시도합니다!"; }
};
