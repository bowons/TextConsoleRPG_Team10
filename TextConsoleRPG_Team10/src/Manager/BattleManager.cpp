#include "../../include/Manager/BattleManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Unit/NormalMonster.h"
#include "../../include/Unit/Boss.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Item/MonsterSpawnData.h"
#include <iostream>
#include <tuple>
#include <memory>
#include <Windows.h>

bool BattleManager::StartAutoBattle(Player* P)
{
    if (!P) return false;

    DataManager* dm = DataManager::GetInstance();
    auto [stage, monsterName] = dm->GetRandomStageAndMonster();
    if (stage.empty() || monsterName.empty()) return false;

    auto NM = std::make_unique<NormalMonster>(P->GetLevel(), stage, monsterName);
    ICharacter* Target = NM.get();
    PrintManager::GetInstance()->PrintLogLine(NM->GetStage());
    PrintManager::GetInstance()->PrintLogLine(Target->GetName() + "이(가) 출현했습니다.. ");

    while (true)
    {
        // === 플레이어 턴 ===
        ProcessTurn(P, Target);
        if (Target->IsDead())
        {
            // 플레이어 승리
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) " + Target->GetName() + "를 쓰러뜨렸습니다!!");
            CalculateReward(P, dynamic_cast<IMonster*>(Target));
            P->ResetBuffs();
            return true;
        }
        
        // === 몬스터 턴 ===
        ProcessAttack(Target, P);
        if (P->IsDead())
        {
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) 패배했습니다...");
            P->ResetBuffs();
            return false;
        }
        
        // === 라운드 종료 ===
        P->ProcessRoundEnd();
    }

}

void BattleManager::ProcessTurn(ICharacter* Atk, ICharacter* Def)
{
    // 자동 아이템 사용: Atk가 Player일 경우에는 인벤토리 검사
    Player* _Player = dynamic_cast<Player*>(Atk);
    if (_Player)
    {
        if (Def->GetCurrentHP() < _Player->GetTotalAtk())
        {
            ProcessAttack(Atk, Def);
            return;
        }
        // 체력이 1/4이하라면 회복 포션 사용 시도
        if (_Player->GetCurrentHP() <= _Player->GetMaxHP() / 4)
        {
            int Slot = _Player->GetInventory().FindFirstSlotIndexOfType<HealPotion>();
            if (Slot != -1) // 회복 포션 탐색 성공
            {
                PrintManager::GetInstance()->PrintLogLine(_Player->GetName() + "이(가) 위험을 감지하고 회복포션을 사용합니다.");
                PrintManager::GetInstance()->PrintLogLine("현재 체력: "+ std::to_string(Def->GetCurrentHP()) + "/" + std::to_string(Def->GetMaxHP()));
                _Player->UseItem(Slot);
                return;  // 체력 물약은 사용 후 바로 턴 종료
            }
        }

        // 상대 HP가 내 공격력보다 높다면 공격력 포션 사용 시도
        if (Def->GetCurrentHP() > _Player->GetTotalAtk())  // GetAtk() -> GetTotalAtk()
        {
            int Slot = _Player->GetInventory().FindFirstSlotIndexOfType<AttackUp>();
            if (Slot != -1) // 공격력 포션 탐색 성공
            {
                PrintManager::GetInstance()->PrintLogLine(_Player->GetName() + "이(가) 전술적으로 공격력 포션을 사용합니다.");
                PrintManager::GetInstance()->PrintLogLine("공격력: " + std::to_string(_Player->GetAtk()));
                _Player->UseItem(Slot);

                // 포션 적용후 바로 공격하거나 턴 끝내는 처리는 알아서
                ProcessAttack(Atk, Def);

                return;
            }
        }

        // 일반 공격
        ProcessAttack(Atk, Def);
    }
    else
    {
        // Atk가 Player가 아닌 경우(몬스터 등)에는 아이템 사용 없음
        ProcessAttack(Atk, Def);
    }
}

void BattleManager::ProcessAttack(ICharacter* Atk, ICharacter* Def)
{
    // 실제 피해 계산은 Attack 구현체가 담당
    int _BeforeHP = Def->GetCurrentHP();
    Atk->Attack(Def);
    int Damage = _BeforeHP - Def->GetCurrentHP();

    // 연출 출력
    PrintManager::GetInstance()->PrintLogLine(Atk->GetAttackNarration());

    // 버프 정보 포함한 피해 로그
    Player* _Player = dynamic_cast<Player*>(Atk);
    std::string damageInfo = " | 피해: " + std::to_string(Damage);
    
    // 플레이어가 버프 상태라면 표시
    if (_Player && _Player->GetTotalAtk() > _Player->GetAtk())
    {
        int BuffBonus = _Player->GetTotalAtk() - _Player->GetAtk();
        damageInfo += " (버프 +" + std::to_string(BuffBonus) + ")";
    }

    std::string msg = Atk->GetName() + "의 공격" + damageInfo
        + " | 남은 체력[" + Def->GetName() + "]: " + std::to_string(Def->GetCurrentHP()) + "/" + std::to_string(Def->GetMaxHP());
    PrintManager::GetInstance()->PrintLogLine(msg);
    Sleep(30);
}

void BattleManager::CalculateReward(Player* P, IMonster* M)
{
    if (!P || !M)
        return;

    auto Reward = M->DropReward(); // tuple<int exp, int gold, unique_ptr<IItem> item>
    int Exp = std::get<0>(Reward);
    int Gold = std::get<1>(Reward);
    std::unique_ptr<IItem> DroppedItem = std::move(std::get<2>(Reward));

    PrintManager::GetInstance()->EndLine();
    if (Exp > 0)
    {
        PrintManager::GetInstance()->PrintLog(P->GetName() + "은(는) ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(Exp), ETextColor::LIGHT_GREEN);
        PrintManager::GetInstance()->PrintLogLine("의 경험치를 획득했습니다.");
        P->GainExp(Exp);
        PrintManager::GetInstance()->PrintLogLine(P->GetName() + "의 EXP: " + std::to_string(P->GetExp()) + "/" + std::to_string(P->GetMaxExp()));
        PrintManager::GetInstance()->EndLine();
    }

    if (Gold > 0)
    {
        PrintManager::GetInstance()->PrintLog(P->GetName() + "은(는) ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(Gold), ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("G를 획득했습니다.");
        P->GainGold(Gold);
        PrintManager::GetInstance()->PrintLog(P->GetName() + "의 소지 골드량은 ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(P->GetGold()) + " G", ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("입니다.");
    }

    if (DroppedItem)
    {
        std::string ItemName = DroppedItem->GetName();
        int Remain;
    
        if (P->GetInventory().AddItem(std::move(DroppedItem), 1, Remain))
        {
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "은(는) " + ItemName + "을 보상으로 얻었습니다.", ELogImportance::DISPLAY);
        }
        else
        {
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "은(는) 인벤토리가 가득 차 있어 아이템을 얻지 못했습니다.", ELogImportance::WARNING);
        }
        PrintManager::GetInstance()->PrintLogLine("");
    }
}

bool BattleManager::StartBossBattle(Player* P)
{
    if (!P) return false;

    std::unique_ptr<Boss> boss = std::make_unique<Boss>(P->GetLevel());

    ICharacter* Target = boss.get();
    PrintManager::GetInstance()->PrintLogLine("보스 출현");
    PrintManager::GetInstance()->PrintLogLine(boss->GetStage());
    PrintManager::GetInstance()->PrintLogLine(Target->GetName() + "이(가) 출현했습니다.. ");

    while (true)
    {
        // === 플레이어 턴 ===
        ProcessTurn(P, Target);
        if (Target->IsDead())
        {
            // 플레이어 승리
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) " + Target->GetName() + "를 쓰러뜨렸습니다!!");
            P->ResetBuffs();
            return true;
        }
        
        // === 몬스터 턴 ===
        ProcessAttack(Target, P);
        if (P->IsDead())
        {
            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) 패배했습니다...");
            P->ResetBuffs();
            return false;
        }
        
        // === 라운드 종료 ===
        P->ProcessRoundEnd();
    }
}