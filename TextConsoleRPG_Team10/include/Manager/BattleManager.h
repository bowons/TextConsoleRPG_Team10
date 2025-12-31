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
    bool StartAutoBattle(Player* p);
    void ProcessTurn(ICharacter* atk, ICharacter* def);
    void CalculateReward(Player* p);
};
