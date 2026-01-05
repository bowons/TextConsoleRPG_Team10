#include "../../include/Manager/BattleManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Unit/NormalMonster.h"
#include "../../include/Unit/Boss.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/Inventory.h"
#include "../../include/Unit/Player.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Item/MonsterSpawnData.h"
#include <iostream>
#include <tuple>
#include <memory>
#include <algorithm>
#include <Windows.h>
#include <optional>


// ========================================
// ===== 레거시 인터페이스 (참고용) =====
// ========================================
// 과거 구현 담당자를 위한 레거시 코드
// Scene 기반 시스템으로 전환 예정

//bool BattleManager::StartAutoBattle(Player* P)
//{
//    if (!P) return false;
//
//    DataManager* dm = DataManager::GetInstance();
//    auto [stage, monsterName] = dm->GetRandomStageAndMonster();
//    if (stage.empty() || monsterName.empty()) return false;
//
//    auto NM = std::make_unique<NormalMonster>(P->GetLevel(), stage, monsterName);
//    ICharacter* Target = NM.get();
//    PrintManager::GetInstance()->PrintLogLine(NM->GetStage());
//    PrintManager::GetInstance()->PrintLogLine(Target->GetName() + "이(가) 출현했습니다.. ");
//
//    while (true)
//    {
//        // === 플레이어 턴 ===
//        ProcessTurn(P, Target);
//        if (Target->IsDead())
//        {
//            // 플레이어 승리
//            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) " + Target->GetName() + "를 쓰러뜨렸습니다!!");
//            CalculateReward(P, dynamic_cast<IMonster*>(Target));
//            P->ResetBuffs();
//            return true;
//        }
//
//        // === 몬스터 턴 ===
//        ProcessAttack(Target, P);
//        if (P->IsDead())
//        {
//            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) 패배했습니다...");
//            P->ResetBuffs();
//            return false;
//        }
//
//        // === 라운드 종료 ===
//        P->ProcessRoundEnd();
//    }
//}

//bool BattleManager::StartBossBattle(Player* P)
//{
//    if (!P) return false;
//
//    std::unique_ptr<Boss> boss = std::make_unique<Boss>(P->GetLevel());
//
//    ICharacter* Target = boss.get();
//    PrintManager::GetInstance()->PrintLogLine("보스 출현");
//    PrintManager::GetInstance()->PrintLogLine(boss->GetStage());
//    PrintManager::GetInstance()->PrintLogLine(Target->GetName() + "이(가) 출현했습니다.. ");
//
//    while (true)
//    {
//        // === 플레이어 턴 ===
//        ProcessTurn(P, Target);
//        if (Target->IsDead())
//        {
//            // 플레이어 승리
//            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) " + Target->GetName() + "를 쓰러뜨렸습니다!!");
//            P->ResetBuffs();
//            return true;
//        }
//
//        // === 몬스터 턴 ===
//        ProcessAttack(Target, P);
//        if (P->IsDead())
//        {
//            PrintManager::GetInstance()->PrintLogLine(P->GetName() + "이(가) 패배했습니다...");
//            P->ResetBuffs();
//            return false;
//        }
//
//        // === 라운드 종료 ===
//        P->ProcessRoundEnd();
//    }
//}

void BattleManager::ProcessTurn(ICharacter* Atk, ICharacter* Def)
{
    // TODO: 팀원 구현 필요 (파티 기반 전투, 직업별 우선순위 등)
    // [Archer → Priest → Warrior → Mage]

    bool turnConsumed = ProcessReservedItems();
    if (turnConsumed)
        return; // 이 캐릭터 턴 종료

    
    //// 현재는 레거시 코드 유지 (참고용)

    //// 자동 아이템 사용: Atk가 Player일 경우에는 인벤토리 검사
    //Player* _Player = dynamic_cast<Player*>(Atk);
    //if (_Player)
    //{
    //    if (Def->GetCurrentHP() < _Player->GetTotalAtk())
    //    {
    //        ProcessAttack(Atk, Def);
    //        return;
    //    }

    //    // 인벤토리 접근 (bool + 포인터)
    //    Inventory* inventory = nullptr;
    //    if (!_Player->TryGetInventory(inventory))
    //    {
    //        ProcessAttack(Atk, Def);
    //        return;
    //    }

    //    // 체력이 1/4이하라면 회복 포션 사용 시도
    //    if (_Player->GetCurrentHP() <= _Player->GetMaxHP() / 4)
    //    {
    //        int Slot = inventory->FindFirstSlotIndexOfType<HealPotion>();
    //        if (Slot != -1) // 회복 포션 탐색 성공
    //        {
    //            PrintManager::GetInstance()->PrintLogLine(_Player->GetName() + "이(가) 위험을 감지하고 회복포션을 사용합니다.");
    //            PrintManager::GetInstance()->PrintLogLine("현재 체력: " + std::to_string(_Player->GetCurrentHP()) + "/" + std::to_string(_Player->GetMaxHP()));
    //            _Player->UseItem(Slot);
    //            return;  // 체력 물약은 사용 후 바로 턴 종료
    //        }
    //    }

    //    // 상대 HP가 내 공격력보다 높다면 공격력 포션 사용 시도
    //    if (Def->GetCurrentHP() > _Player->GetTotalAtk())
    //    {
    //        int Slot = inventory->FindFirstSlotIndexOfType<AttackUp>();
    //        if (Slot != -1) // 공격력 포션 탐색 성공
    //        {
    //            PrintManager::GetInstance()->PrintLogLine(_Player->GetName() + "이(가) 전술적으로 공격력 포션을 사용합니다.");
    //            PrintManager::GetInstance()->PrintLogLine("공격력: " + std::to_string(_Player->GetAtk()));
    //            _Player->UseItem(Slot);
    //            ProcessAttack(Atk, Def);
    //            return;
    //        }
    //    }

    //    // 일반 공격
    //    ProcessAttack(Atk, Def);
    //}
    //else
    //{
    //    // Atk가 Player가 아닌 경우(몬스터 등)에는 아이템 사용 없음
    //    ProcessAttack(Atk, Def);
    //}
}

void BattleManager::ProcessAttack(ICharacter* Atk, ICharacter* Def)
{
    // TODO: 팀원 구현 필요 (파티 공격 시스템 등)
    // 현재는 레거시 코드 유지 (참고용)

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
    // TODO: 팀원 구현 필요
    // 레거시 코드를 참고하여 구현하세요

    if (!P || !M)
        return;

    auto Reward = M->DropReward();
    int Exp = std::get<0>(Reward);
    int Gold = std::get<1>(Reward);
    std::unique_ptr<IItem> DroppedItem = std::move(std::get<2>(Reward));

    // TODO: BattleResult에 저장
    _Result.ExpGained = Exp;
    _Result.GoldGained = Gold;
    if (DroppedItem)
        _Result.ItemName = DroppedItem->GetName();

    // TODO: GameManager에서 메인 플레이어 및 파티 가져오기
    // TODO: 경험치 파티 전체 분배
  // TODO: 골드 메인 플레이어에게만 지급
    // TODO: 아이템 메인 플레이어 인벤토리에 추가

    /* ===== 레거시 코드 (참고용) =====

    GameManager* gm = GameManager::GetInstance();
    std::shared_ptr<Player> mainPlayer = gm->GetMainPlayer();
    const auto& party = gm->GetParty();

    PrintManager::GetInstance()->EndLine();

    // ===== 경험치: 모든 동료가 동일 수치로 획득 =====
    if (Exp > 0)
    {
        if (party.size() > 1)
        {
  PrintManager::GetInstance()->PrintLog("파티원 모두 ");
      }
        else
        {
       PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "은(는) ");
        }
        PrintManager::GetInstance()->PrintColorText(std::to_string(Exp), ETextColor::LIGHT_GREEN);
      PrintManager::GetInstance()->PrintLogLine("의 경험치를 획득했습니다.");

        // 파티 전체에 경험치 분배
   for (const auto& member : party)
        {
     if (member)
   {
                member->GainExp(Exp);
        PrintManager::GetInstance()->PrintLogLine(
         member->GetName() + "의 EXP: " +
                std::to_string(member->GetExp()) + "/" +
      std::to_string(member->GetMaxExp()));
     }
        }
        PrintManager::GetInstance()->EndLine();
    }

    // ===== 골드: 메인 플레이어만 획득 =====
    if (Gold > 0 && mainPlayer)
    {
        PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "은(는) ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(Gold), ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("G를 획득했습니다.");
    mainPlayer->GainGold(Gold);
        PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "의 소지 골드량은 ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(mainPlayer->GetGold()) + " G", ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("입니다.");
 }

    // ===== 아이템: 메인 플레이어 인벤토리에만 추가 =====
    if (DroppedItem && mainPlayer)
    {
        std::string ItemName = DroppedItem->GetName();

        // 메인 플레이어 인벤토리 접근
        Inventory* inventory = nullptr;
    if (!mainPlayer->TryGetInventory(inventory))
        {
        PrintManager::GetInstance()->PrintLogLine(
        mainPlayer->GetName() + "은(는) 인벤토리가 없어 아이템을 얻지 못했습니다.",
        ELogImportance::WARNING);
        PrintManager::GetInstance()->PrintLogLine("");
        return;
        }

    int Remain;
        if (inventory->AddItem(std::move(DroppedItem), 1, Remain))
        {
    PrintManager::GetInstance()->PrintLogLine(
           mainPlayer->GetName() + "은(는) " + ItemName + "을 보상으로 얻었습니다.",
  ELogImportance::DISPLAY);
      }
        else
        {
   PrintManager::GetInstance()->PrintLogLine(
      mainPlayer->GetName() + "은(는) 인벤토리가 가득 차 있어 아이템을 얻지 못했습니다.",
            ELogImportance::WARNING);
        }
     PrintManager::GetInstance()->PrintLogLine("");
    }

    ===== 레거시 코드 끝 ===== */
}

// ========================================
// ===== Scene 기반 신규 인터페이스 =====
// ========================================

bool BattleManager::StartBattle(EBattleType type, int Floor)
{
    // 1. 이미 전투 중이면 실패
    if (_IsBattleActive)
        return false;

    DataManager* dm = DataManager::GetInstance();
    std::optional<MonsterSpawnData> monsterOpt;

    // 2. 타입별 데이터 로드
    switch (type)
    {
    case EBattleType::Normal:
        monsterOpt = dm->GetMonster("Enemy_Normal.csv", Floor);
        break;

    case EBattleType::Elite:
        monsterOpt = dm->GetMonster("Enemy_Elite.csv", Floor);
        break;

    case EBattleType::Boss:
        monsterOpt = dm->GetMonster("Enemy_Boss.csv", Floor);
        break;
    }

    // 2. 데이터 없으면 실패
    if (!monsterOpt.has_value())
    {
        std::cout << "해당 층에는 몬스터가 존재하지 않습니다.\n";
        return false;
    }

    const MonsterSpawnData& data = monsterOpt.value();

    // 3. 몬스터 생성
    if (type == EBattleType::Boss)
    {
        _CurrentMonster = std::make_unique<Boss>(data);
    }
    else
    {
        // ✅ Normal + Elite 공용
        _CurrentMonster = std::make_unique<NormalMonster>(data);
    }

    // 4. 전투 상태 초기화
    _BattleType = type;
    _IsBattleActive = true;
    _Result = BattleResult{};

    _CurrentRound = 0;
    _ItemReservations.clear();

    return true;
}

void BattleManager::EndBattle()
{
    // TODO: 팀원 구현 필요
    // 1. 전투 중이 아니면 즉시 반환
    if(!_IsBattleActive)
        return;
    // 2. GameManager에서 파티 가져오기
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();
    // 3. 모든 파티원 버프 초기화 (ResetBuffs)
    for (const auto& member : party)
    {
        if (member)
            member->ResetTempStats();
    }
    // 4. 승리 시 CalculateReward 호출
    if (_Result.Victory)
    {
        Player* mainPlayer = gm->GetMainPlayer().get();
        CalculateReward(mainPlayer, _CurrentMonster.get());
    }
    // 5. GameManager::RemoveDeadCompanions() 호출
    gm->RemoveDeadCompanions();
    // 6. 예약 목록 정리: 모든 예약된 아이템 CancelReservation() 호출
    for (auto& reservation : _ItemReservations)
    {
        if (!reservation.IsActive)
            continue;

        Player* user = reservation.User;
        if (!user)
            continue;

        Inventory* inventory = nullptr;
        if (!user->TryGetInventory(inventory))
            continue;

        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (item)
        {
            item->CancelReservation();
        }

        reservation.IsActive = false;
    }

    _ItemReservations.clear();
    _CurrentRound = 0;
    _CurrentMonster.reset();
    _BattleType = EBattleType::None;
    _IsBattleActive = false;
}

bool BattleManager::ProcessBattleTurn()
{
    // TODO: 팀원 구현 필요
    // 1. 전투 중이 아니거나 몬스터가 없으면 false 반환
    if(!_IsBattleActive || !_CurrentMonster)
        return false;
    // 2. _CurrentRound++ (라운드 증가)
    SetCurrentRound(_CurrentRound + 1);
    // 3. ProcessReservedItems() 호출 (예약된 아이템 자동 처리)
    ProcessReservedItems();
    // 4. GameManager에서 메인 플레이어 가져오기
    GameManager* gm = GameManager::GetInstance();
    Player* mainPlayer = gm->GetMainPlayer().get();
    // 5. 플레이어 턴: ProcessTurn(Player, Monster)
    
    // 6. 몬스터 사망 확인
    //- 사망 시: _Result.Victory = true, IsCompleted = true, 승리 메시지 출력, false 반환
    // 7. 몬스터 턴: ProcessAttack(Monster, Player)
    // 8. 플레이어 사망 확인
    //    - 사망 시: _Result.Victory = false, IsCompleted = true, 패배 메시지 출력, false 반환
    // 9. 라운드 종료: 파티 전체 ProcessRoundEnd()
    // 10. 전투 계속: true 반환

    return false;  // TODO: 구현 후 실제 결과 반환
}

// ========================================
// ===== 아이템 예약 시스템 =====
// ========================================

bool BattleManager::ReserveItemUse(Player* player, int slotIndex)
{
    if (!player) {
        PrintManager::GetInstance()->PrintLogLine("플레이어가 유효하지 않습니다.", ELogImportance::WARNING);
        return false;
    }

    // 인벤토리 확인
    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory)) {
        PrintManager::GetInstance()->PrintLogLine("인벤토리가 없습니다.", ELogImportance::WARNING);
        return false;
    }

    // 슬롯 유효성 검증
    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item) {
        PrintManager::GetInstance()->PrintLogLine("해당 슬롯에 아이템이 없습니다.", ELogImportance::WARNING);
        return false;
    }

    // 이미 예약되어 있는지 확인
    if (item->IsReserved()) {
        PrintManager::GetInstance()->PrintLogLine(
            item->GetName() + "은(는) 이미 예약되어 있습니다.",
            ELogImportance::WARNING
        );
        return false;
    }

    // 예약 등록
    item->Reserve(_CurrentRound);
    _ItemReservations.push_back({ slotIndex, player, true });

    PrintManager::GetInstance()->PrintLogLine(
        item->GetName() + " 사용 예약 완료! (조건: " +
        item->GetUseConditionDescription() + ")",
        ELogImportance::DISPLAY
    );

    return true;
}

bool BattleManager::CancelItemReservation(Player* player, int slotIndex)
{
    if (!player) return false;

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory)) return false;

    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item || !item->IsReserved()) {
        PrintManager::GetInstance()->PrintLogLine(
            "해당 슬롯에 예약된 아이템이 없습니다.",
            ELogImportance::WARNING
        );
        return false;
    }

    // 예약 목록에서 제거
    for (auto& reservation : _ItemReservations) {
        if (reservation.User == player &&
            reservation.SlotIndex == slotIndex &&
            reservation.IsActive) {

            reservation.IsActive = false;
            item->CancelReservation();

            PrintManager::GetInstance()->PrintLogLine(
                item->GetName() + " 예약이 취소되었습니다.",
                ELogImportance::DISPLAY
            );
            return true;
        }
    }

    return false;
}

void BattleManager::ProcessReservedItems()
{
    if (_ItemReservations.empty()) return;

    PrintManager::GetInstance()->EndLine();
    PrintManager::GetInstance()->PrintLogLine("=== 예약된 아이템 처리 중 ===", ELogImportance::DISPLAY);

    // 활성화된 예약만 처리
    for (auto& reservation : _ItemReservations) {
        if (!reservation.IsActive) continue;

        Player* user = reservation.User;
        Inventory* inventory = nullptr;

        if (!user->TryGetInventory(inventory)) continue;

        // 아이템 가져오기
        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (!item) {
            // 아이템이 사라짐 → 예약 취소
            PrintManager::GetInstance()->PrintLogLine(
                "슬롯 [" + std::to_string(reservation.SlotIndex) + "]의 아이템이 없어져 예약이 취소되었습니다.",
                ELogImportance::WARNING
            );
            reservation.IsActive = false;
            continue;
        }

        // ===== 조건 체크 (IItem::CanUse) =====
        if (!item->CanUse(*user, _CurrentRound)) {
            // 조건 불만족 → 예약 유지 (다음 턴 재시도)
            PrintManager::GetInstance()->PrintLogLine(
                item->GetName() + " - 조건 미달 (" +
                item->GetUseConditionDescription() + ") → 대기 중",
                ELogImportance::DISPLAY
            );
            continue;
        }

        // ===== 조건 만족 → 자동 사용 =====
        PrintManager::GetInstance()->PrintLogLine(
            ">>> " + user->GetName() + "의 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)",
            ELogImportance::DISPLAY
        );

        // 효과 적용
        item->ApplyEffect(*user);

        // 인벤토리에서 제거
        inventory->RemoveItem(reservation.SlotIndex, 1);

        // 예약 취소
        item->CancelReservation();

        // 예약 완료 → 비활성화
        reservation.IsActive = false;
    }

    // 비활성화된 예약 정리
    _ItemReservations.erase(
        std::remove_if(_ItemReservations.begin(), _ItemReservations.end(),
            [](const ItemReservation& r) { return !r.IsActive; }),
        _ItemReservations.end()
    );

    PrintManager::GetInstance()->EndLine();
}