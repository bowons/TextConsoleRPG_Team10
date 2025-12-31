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
    int _Atk;

public:
    virtual void TakeDamage(int amount) = 0;
    virtual void Attack(ICharacter* target) = 0;
    virtual bool IsDead() = 0;
    virtual ~ICharacter() {}

    inline const::string& GetName() const { return _Name; }
    inline int GetLevel() const { return _Level; }
    inline int GetCurrentHP() const { return _CurrentHP; }
    inline int GetMaxHP() const { return _MaxHP; }
    inline int GetAtk() const { return _Atk; }
};
