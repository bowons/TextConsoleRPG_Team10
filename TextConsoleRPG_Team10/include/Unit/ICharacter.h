#pragma once
#include <string>

class ICharacter
{
protected:
    std::string _Name;
    int _Level;
    int _CurrentHP;
    int _MaxHP;
    int _Atk;

public:
    virtual ~ICharacter() {}

    virtual void TakeDamage(const int Amount) = 0;
    virtual void Attack(ICharacter* Target) const = 0;
    virtual bool IsDead() const = 0;

    inline const std::string& GetName() const { return _Name; }
    inline int GetLevel() const { return _Level; }
    inline int GetCurrentHP() const { return _CurrentHP; }
    inline int GetMaxHP() const { return _MaxHP; }
    inline int GetAtk() const { return _Atk; }

    // 공격 연출 문자열 반환 (기본값)
    virtual std::string GetAttackNarration() const { return _Name + "이(가) 공격을 시도합니다!"; }
};
