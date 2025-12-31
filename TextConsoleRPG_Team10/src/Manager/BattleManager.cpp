#include "../../include/Manager/BattleManager.h"
#include "../../include/Manager/PrintManager.h"
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
            // 전투 결과에 따른 보상 계산 및 지급, 플레이어는 경험치 및 골드를 획득합니다.
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
    // TODO: 전투 턴을 진행함
    //아이템 사용 여부 체크 후 사용

    // 플레이어의 아이템 사용 조건
    // 회복 포션: 체력이 1/4 이하일 때 사용
    // 공격력 포션: Def의 체력이 Atk의 공격력 보다 높다면 조건 진입
    // Atk가 공격력 포션을 보유하고 있으며, Atk의 공격력 포션 사용을 가정해도 Def의 체력이 Atk의 공격력 보다 높다면 사용
    
    if(Atk->GetCurrentHP() < Atk->GetMaxHP() / 4) 
    {
        Player* P = dynamic_cast<Player*>(Atk);
        if(P != nullptr)
        {
            // TODO: 인벤토리에서 회복 포션 슬롯 인덱스 찾기
            
        }
    }

    if (Def->GetCurrentHP() > Atk->GetAtk())
    {
        Player* P = dynamic_cast<Player*>(Atk);
        if (P != nullptr)
        {
            // TODO: 인벤토리에서 공격력 증가 포션 슬롯 인덱스 찾기

        }

        return;
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