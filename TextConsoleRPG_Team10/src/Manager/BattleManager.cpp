#include "../../include/Manager/BattleManager.h"
#include "../../include/Unit/NormalMonster.h"
#include "../../include/Unit/Player.h"
#include <iostream>
#include <tuple>
#include <memory>

using namespace std;

bool BattleManager::StartAutoBattle(Player* P)
{
    // Implementation needed
    // 현재: 플레이어 선공, 노멀 몬스터로 고정 (추후 몬스터 지정 필요)
    NormalMonster* NM = new NormalMonster(P->GetLevel());

    while (true)
    {
        ProcessTurn(P, NM);
        if (NM->IsDead())
        {
            //플레이어 승리
            CalculateReward(P, NM);
            return true;
        }
        ProcessAttack(NM, P);
        if (P->IsDead())
        {
            //플레이어 패배
            cout << "플레이어 패배" << endl;
            return false;
        }
    }

}

void BattleManager::ProcessTurn(ICharacter* Atk, ICharacter* Def)
{
    //아이템 사용 여부 체크 후 사용
    if(Atk->GetCurrentHP() < Atk->GetMaxHP() / 4) //dummy // 체력 1/4 이하일 때 아이템 사용 
    {
        Player* P = dynamic_cast<Player*>(Atk);
        if(P != nullptr)
        {
            // 아이템 미보유 예외처리 추가 필요
            P->UseItem(0); //dummy
            cout << P->GetName() << "이(가) 아이템을 사용했습니다." << endl;
        }
    }
    ProcessAttack(Atk, Def);
}

void BattleManager::ProcessAttack(ICharacter* Atk, ICharacter* Def)
{
    Atk->Attack(Def);
    cout << Atk->GetName() << "의 공격" << endl;
    cout << Def->GetName() << "의 피해량: " << Atk->GetAtk() << ", 남은 체력: " << Def->GetCurrentHP() << "/" << Def->GetMaxHP() << endl;
}

void BattleManager::CalculateReward(Player* P, IMonster* M)
{
    // Implementation needed
    // 전투 결과에 따른 보상 계산 및 지급, 플레이어는 경험치 및 골드, 아이템 획득

    tuple<int, int, IItem*>Reward = M->DropReward();
    P->GainExp(get<0>(Reward)); // 경험치 획득
    P->GainGold(get<1>(Reward)); // 골드 획득
    //P->GetInventory().AddItem(아이템포인터, 1);
}