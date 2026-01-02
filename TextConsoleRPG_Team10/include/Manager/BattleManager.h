#pragma once
#include "../Singleton.h"

class Player;
class IMonster;
class ICharacter;

class BattleManager : public Singleton<BattleManager>
{
private:
    IMonster* _CurrentMonster;

public:
    bool StartAutoBattle(Player* P);
    void ProcessTurn(ICharacter* Atk, ICharacter* Def);
    void ProcessAttack(ICharacter* Atk, ICharacter* Def);
    void CalculateReward(Player* P, IMonster* M);
};
