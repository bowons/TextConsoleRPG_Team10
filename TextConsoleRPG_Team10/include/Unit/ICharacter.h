#pragma once
#include <string>

using namespace std;

class ICharacter
{
protected:
    string _Name;
    int _Level;
    int _CurrentHP;
    int _MaxHP;
    int _BaseAtk;
    int _CurrentAtk;

public:
    virtual ~ICharacter() {}

    virtual void TakeDamage(const int Amount) = 0;
    virtual void Attack(ICharacter* Target) const = 0;
    virtual bool IsDead() const = 0;

	inline const string& GetName() const { return _Name; }
    inline int GetLevel() const { return _Level; }
    inline int GetCurrentHP() const { return _CurrentHP; }
    inline int GetMaxHP() const { return _MaxHP; }
    inline int GetBaseAtk() const { return _BaseAtk; }
    inline int GetCurrentAtk() const { return _CurrentAtk; }
};
