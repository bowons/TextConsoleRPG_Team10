#pragma once
#include "ICharacter.h"
#include "../Item/Inventory.h"
#include <string>

using namespace std;

class Player : public ICharacter
{
private:
    int _CurrentExp;
    int _MaxExp;
    int _Gold;
    Inventory _Inventory;

public:
    Player(const string& name);
    void TakeDamage(int amount) override;
    void Attack(ICharacter* target) override;
    bool IsDead() override;
    
    void CheckLevelUp();
    void ProcessLevelUp();
    void GainExp(int amount);
    void GainGold(int amount);
    void UseItem(int slotIndex);
};
