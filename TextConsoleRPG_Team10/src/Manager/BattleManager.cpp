#include "../../include/Manager/BattleManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Unit/NormalMonster.h"
#include "../../include/Unit/EliteMonster.h"  // 추가
#include "../../include/Unit/Boss.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Item/IItem.h"
#include "../../include/Item/Inventory.h"
#include "../../include/Unit/Player.h"
#include "../../include/Unit/Warrior.h"
#include "../../include/Unit/Mage.h"
#include "../../include/Unit/Archer.h"
#include "../../include/Unit/Priest.h"
#include "../../include/Skill/ISkill.h"  // SkillResult 포함
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Data/FloorScalingData.h"  // 추가
#include <iostream>
#include <tuple>
#include <memory>
#include <algorithm>
#include <Windows.h>
#include <optional>
#include "../../include/UI/Scenes/BattleScene.h"

// TODO: 로그 찍기, CalculateReward 내부 구현

// 코드 흐름, 각 함수 호출은 GM에서 이루어짐?
//StartBattle / ProcessBattleTurn -> 플레이어 ProcessTurn -> ProcessAttack -> 몬스터 ProcessAttack / EndBattle -> CalculateReward


// ===== 몬스터 타겟 선정 (어그로 최댓값 기준) =====
Player* BattleManager::SelectMonsterTarget()
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    Player* target = nullptr;
    int maxAggro = -1;

    // 생존한 파티원 중 어그로 최댓값 찾기
    for (const auto& member : party)
    {
        if (!member || member->IsDead())
            continue;

        int aggro = member->GetAggro();
        if (aggro > maxAggro)
        {
            maxAggro = aggro;
            target = member.get();
        }
    }

    // 어그로 동점이거나 타겟 없으면 메인 플레이어
    return target ? target : gm->GetMainPlayer().get();
}

void BattleManager::ProcessTurn(ICharacter* Def)
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    // 정렬용 복사본 (shared_ptr -> raw pointer)
    std::vector<ICharacter*> sortedParty;
    for (const auto& member : party)
    {
        if (member)
            sortedParty.push_back(member.get());
    }

    // ===== 직업 우선순위 정렬 (1순위: 직업, 2순위: DEX) =====
    std::sort(sortedParty.begin(), sortedParty.end(),
        [](ICharacter* a, ICharacter* b)
        {
            int jobA = GetJobPriority(a);
            int jobB = GetJobPriority(b);

            if (jobA != jobB)
            {
                return jobA < jobB;  // 직업 우선순위
            }

            // 같은 직업이면 DEX 높은 순
            return a->GetDex() > b->GetDex();
        });

    // TODO: BattleScene에서 "=== 플레이어 턴 시작 ===" 로그 표시
    /*PrintManager::GetInstance()->EndLine();
    PrintManager::GetInstance()->PrintLogLine(
        "=== 플레이어 턴 시작 ===",
        ELogImportance::DISPLAY
    );*/
    //PushLog("=== 플레이어 턴 시작 ===", EBattleLogType::Important);

    // 파티 돌아가면서 행동 진행
    for (ICharacter* member : sortedParty)
    {
        if (!member || member->IsDead())
            continue;

        Player* player = dynamic_cast<Player*>(member);
        if (!player)
            continue;

        // 1️⃣ 해당 플레이어의 예약 아이템 체크
        if (TryUseReservedItem(player))
        {
            // 아이템 사용으로 턴 종료
            /*PrintManager::GetInstance()->PrintLogLine(
                player->GetName() + "은(는) 아이템 사용으로 턴 종료",
                ELogImportance::DISPLAY
            );*/
            PushLog(player->GetName() + "은(는) 아이템 사용으로 턴 종료", EBattleLogType::Important);
            continue;
        }

        // 2️⃣ 아이템 미사용 시 공격 진행
        ProcessAttack(member, Def);
    }
}

void BattleManager::ProcessAttack(ICharacter* Atk, ICharacter* Def)
{
    if (!Atk || !Def)
        return;

    // Player의 경우 스킬 우선 사용 시도
    if (Player* player = dynamic_cast<Player*>(Atk))
    {
        int skillIndex = player->SelectBestSkill(Def);
        if (skillIndex >= 0)
        {
            SkillResult result = player->UseSkill(skillIndex, Def);
            if (result.Success)
            {
                // MP 소모 추적
                player->TrackMPSpent(player->GetSkills()[skillIndex]->GetMPCost());

                /*PrintManager::GetInstance()->PrintLogLine(
                    player->GetName() + "의 " + result.SkillName + "!",
                    ELogImportance::DISPLAY
                );*/
                PushLog("'"+player->GetName() + "'의 " + result.SkillName + "!", EBattleLogType::Important);

                // 다단 히트 처리
                if (result.HitCount > 1)
                {
                    int totalDamage = 0;
                    for (int i = 0; i < result.HitCount; ++i)
                    {
                        int damage = Def->TakeDamage(player, result.Value);
                        totalDamage += damage;

                        // 어그로 증가 (공격 시 +10)
                        player->ModifyAggro(10);

                        /*PrintManager::GetInstance()->PrintLogLine(
                            "타격 " + std::to_string(i + 1) + "/" + std::to_string(result.HitCount) +
                            ": " + std::to_string(damage) + " 데미지!",
                            ELogImportance::DISPLAY
                        );*/
                        PushLog("타격 " + std::to_string(i + 1) + "/" + std::to_string(result.HitCount) +
                            ": " + std::to_string(damage) + " 데미지!", EBattleLogType::Important);

                        if (Def->IsDead())
                            break;
                    }

                    /*PrintManager::GetInstance()->PrintLogLine(
                        "총 데미지: " + std::to_string(totalDamage),
                        ELogImportance::DISPLAY
                    );*/
                    PushLog("총 데미지: " + std::to_string(totalDamage), EBattleLogType::Important);
                }
                else
                {
                    // 단일 타격
                    int damage = Def->TakeDamage(player, result.Value);

                    // 어그로 증가
                    player->ModifyAggro(10);

                    /*PrintManager::GetInstance()->PrintLogLine(
                        Def->GetName() + "에게 " + std::to_string(damage) + " 데미지!",
                        ELogImportance::DISPLAY
                    );*/
                    PushLog(Def->GetName() + "에게 " + std::to_string(damage) + " 데미지!", EBattleLogType::Important);
                }

                if (!result.Message.empty())
                {
                    PrintManager::GetInstance()->PrintLogLine(
                        result.Message,
                        ELogImportance::DISPLAY
                    );
                }
                RequestFlush();
                return;  // 스킬 사용 성공 → 일반 공격 스킵
            }
            // 스킬 사용 실패 (MP 부족 등) → 일반 공격으로 폴백
        }
    }

    // ===== 몬스터 공격 처리 =====
    // 스킬 사용 실패 또는 Monster → 일반 공격
    std::tuple<std::string, int> attackResult = Atk->Attack(Def);
    std::string attackType = std::get<0>(attackResult);
    int baseDamage = std::get<1>(attackResult);

    // ===== Boss 특수 스킬 처리 =====
    if (Boss* boss = dynamic_cast<Boss*>(Atk))
    {
        // 광역 공격 스킬
        if (attackType == "어둠의 폭풍")
        {
            ProcessAOEAttack(attackType, baseDamage, boss);
            RequestFlush();
            return;
        }

        // 디버프 스킬
        if (attackType == "공포의 속삭임")
        {
            ProcessDebuff(attackType, boss);
            RequestFlush();
            return;
        }
    }

    // ===== 일반 단일 공격 =====
    int Damage = Def->TakeDamage(Atk, baseDamage);

    // Player의 일반 공격 시 어그로는 Attack() 내부에서 처리됨 (치명타 판별 포함)

    // 공격 타입에 따른 로그 출력
    bool isCritical = (attackType == "치명타!");
    bool isSpecialSkill = (attackType != "공격" && attackType != "치명타!");

    if (isCritical)
    {
        /*PrintManager::GetInstance()->PrintLogLine(
            "💥 " + Atk->GetName() + "의 " + attackType + " 💥",
            ELogImportance::DISPLAY
        );*/
        PushLog("💥 " + Atk->GetName() + "의 " + attackType + " 💥", EBattleLogType::Important);
    }
    else if (isSpecialSkill)
    {
        /*PrintManager::GetInstance()->PrintLogLine(
            "⚡ " + Atk->GetName() + "의 " + attackType + "! ⚡",
            ELogImportance::DISPLAY
        );*/
        PushLog("⚡ " + Atk->GetName() + "의 " + attackType + "! ⚡", EBattleLogType::Important);
    }
    else
    {
        /*PrintManager::GetInstance()->PrintLogLine(
            Atk->GetName() + "의 " + attackType,
            ELogImportance::DISPLAY
        );*/
        PushLog(Atk->GetName() + "의 " + attackType, EBattleLogType::Important);
    }

   /* PrintManager::GetInstance()->PrintLogLine(
        Def->GetName() + "에게 " + std::to_string(Damage) + " 데미지!",
        ELogImportance::DISPLAY
    );*/
    PushLog(Def->GetName() + "에게 " + std::to_string(Damage) + " 데미지!", EBattleLogType::Important);

    RequestFlush();
}

// ===== 광역 공격 처리 (Boss 전용) =====
void BattleManager::ProcessAOEAttack(const std::string& skillName, int damage, ICharacter* attacker)
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    /*PrintManager::GetInstance()->PrintLogLine(
        "🌪️ " + attacker->GetName() + "의 " + skillName + "! 🌪️",
        ELogImportance::DISPLAY
    );*/
    PushLog("🌪️ " + attacker->GetName() + "의 " + skillName + "! 🌪️", EBattleLogType::Important);

    // 파티 전체에 데미지
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            int actualDamage = member->TakeDamage(attacker, damage);
            /*PrintManager::GetInstance()->PrintLogLine(
                "  → " + member->GetName() + "에게 " + std::to_string(actualDamage) + " 데미지!",
                ELogImportance::DISPLAY
            );*/
            PushLog("  → " + member->GetName() + "에게 " + std::to_string(actualDamage) + " 데미지!", EBattleLogType::Important);
        }
    }
    RequestFlush();
}

// ===== 디버프 처리 (Boss 전용) =====
void BattleManager::ProcessDebuff(const std::string& skillName, ICharacter* attacker)
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    /*PrintManager::GetInstance()->PrintLogLine(
        "😱 " + attacker->GetName() + "의 " + skillName + "! 😱",
        ELogImportance::DISPLAY
    );*/
    PushLog("😱 " + attacker->GetName() + "의 " + skillName + "! 😱", EBattleLogType::Important);

    // 파티 전체에 공격력 디버프 (-30%, 2라운드)
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            int debuffAmount = -static_cast<int>(member->GetAtk() * 0.3f);
            member->ApplyTempAtkBuff(debuffAmount, 2);

            /*PrintManager::GetInstance()->PrintLogLine(
                "  → " + member->GetName() + "의 공격력이 감소했다! (" + std::to_string(debuffAmount) + ", 2라운드)",
                ELogImportance::DISPLAY
            );*/
            PushLog("  → " + member->GetName() + "의 공격력이 감소했다! (" + std::to_string(debuffAmount) + ", 2라운드)", EBattleLogType::Important);
        }
    }
    RequestFlush();
}

void BattleManager::CalculateReward(Player* P, IMonster* M)
{
    if (!P || !M)
        return;

    auto Reward = M->DropReward();
    int Exp = std::get<0>(Reward);
    int Gold = std::get<1>(Reward);
    std::unique_ptr<IItem> DroppedItem = std::move(std::get<2>(Reward));

    // BattleResult에 저장
    _Result.ExpGained = Exp;
    _Result.GoldGained = Gold;
    if (DroppedItem)
        _Result.ItemName = DroppedItem->GetName();

    // ===== 보상 분배 =====
    GameManager* gm = GameManager::GetInstance();
    std::shared_ptr<Player> mainPlayer = gm->GetMainPlayer();
    const auto& party = gm->GetParty();

    PrintManager::GetInstance()->EndLine();

    // ===== 경험치: 모든 동료가 동일 수치로 획득 =====
    if (Exp > 0)
    {
        if (party.size() > 1)
        {
            //PrintManager::GetInstance()->PrintLog("파티원 모두 ");
            PushLog("파티원 모두 ", EBattleLogType::Important);
        }
        else
        {
            //PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "은(는) ");
            PushLog(mainPlayer->GetName() + "은(는) ", EBattleLogType::Important);
        }
        /*PrintManager::GetInstance()->PrintColorText(std::to_string(Exp), ETextColor::LIGHT_GREEN);
        PrintManager::GetInstance()->PrintLogLine("의 경험치를 획득했습니다.");*/
        PushLog(std::to_string(Exp)+ "의 경험치를 획득했습니다.", EBattleLogType::Important);

        // 파티 전체에 경험치 분배
        for (const auto& member : party)
        {
            if (member)
            {
                member->GainExp(Exp);
                /*PrintManager::GetInstance()->PrintLogLine(
                    member->GetName() + "의 EXP: " +
                    std::to_string(member->GetExp()) + "/" +
                    std::to_string(member->GetMaxExp()));*/
                PushLog(member->GetName() + "의 EXP: " +
                    std::to_string(member->GetExp()) + "/" +
                    std::to_string(member->GetMaxExp()), EBattleLogType::Important);
            }
        }
        PrintManager::GetInstance()->EndLine();
    }

    // ===== 골드: 메인 플레이어만 획득 =====
    if (Gold > 0 && mainPlayer)
    {
        /*PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "은(는) ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(Gold), ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("G를 획득했습니다.");*/
        PushLog(mainPlayer->GetName() + "은(는) "+ std::to_string(Gold)+ "G를 획득했습니다.", EBattleLogType::Important);
        mainPlayer->GainGold(Gold);
        /*PrintManager::GetInstance()->PrintLog(mainPlayer->GetName() + "의 소지 골드량은 ");
        PrintManager::GetInstance()->PrintColorText(std::to_string(mainPlayer->GetGold()) + " G", ETextColor::YELLOW);
        PrintManager::GetInstance()->PrintLogLine("입니다.");*/
        PushLog(mainPlayer->GetName() + "의 소지 골드량은 "+ std::to_string(mainPlayer->GetGold()) + " G"+ "입니다.", EBattleLogType::Important);
    }

    // ===== 아이템: 메인 플레이어 인벤토리에만 추가 =====
    if (DroppedItem && mainPlayer)
    {
        std::string ItemName = DroppedItem->GetName();

        // 메인 플레이어 인벤토리 접근
        Inventory* inventory = nullptr;
        if (!mainPlayer->TryGetInventory(inventory))
        {
            /*PrintManager::GetInstance()->PrintLogLine(
                mainPlayer->GetName() + "은(는) 인벤토리가 없어 아이템을 얻지 못했습니다.",
                ELogImportance::WARNING);*/
            PushLog(mainPlayer->GetName() + "은(는) 인벤토리가 없어 아이템을 얻지 못했습니다.", EBattleLogType::Important);
            //PrintManager::GetInstance()->PrintLogLine("");
            PushLog("", EBattleLogType::Important);
            return;
        }

        int Remain;
        if (inventory->AddItem(std::move(DroppedItem), 1, Remain))
        {
            /*PrintManager::GetInstance()->PrintLogLine(
                mainPlayer->GetName() + "은(는) " + ItemName + "을 보상으로 얻었습니다.",
                ELogImportance::DISPLAY);*/
            PushLog(mainPlayer->GetName() + "은(는) " + ItemName + "을 보상으로 얻었습니다.", EBattleLogType::Important);
        }
        else
        {
            /*PrintManager::GetInstance()->PrintLogLine(
                mainPlayer->GetName() + "은(는) 인벤토리가 가득 차 있어 아이템을 얻지 못했습니다.",
                ELogImportance::WARNING);*/
            PushLog(mainPlayer->GetName() + "은(는) 인벤토리가 가득 차 있어 아이템을 얻지 못했습니다.", EBattleLogType::Important);
        }
        //PrintManager::GetInstance()->PrintLogLine("");
        PushLog("", EBattleLogType::Important);
    }
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

    // 3. 데이터 없으면 실패
    if (!monsterOpt.has_value())
    {
        std::cout << "해당 층에는 몬스터가 존재하지 않습니다.\n";
        return false;
    }

    MonsterSpawnData data = monsterOpt.value();

    // ===== 4. FloorScaling 적용 =====
    std::optional<FloorScalingData> scalingOpt = dm->GetFloorScaling(Floor);
    if (scalingOpt.has_value())
    {
        const FloorScalingData& scaling = scalingOpt.value();

        // 스탯에 배율 적용
        data.hp = static_cast<int>(data.hp * scaling.hp_mul);
        data.mp = static_cast<int>(data.mp * scaling.mp_mul);
        data.atk = static_cast<int>(data.atk * scaling.atk_mul);
        data.def = static_cast<int>(data.def * scaling.def_mul);
        data.dex = static_cast<int>(data.dex * scaling.dex_mul);
        data.luk = static_cast<int>(data.luk * scaling.luk_mul);
        data.crit_rate *= scaling.crit_mul;
        data.exp = static_cast<int>(data.exp * scaling.exp_mul);
        data.gold = static_cast<int>(data.gold * scaling.gold_mul);
    }

    // 5. 몬스터 생성 (타입별 분기)
    if (type == EBattleType::Boss)
    {
        _CurrentMonster = std::make_unique<Boss>(data);
    }
    else if (type == EBattleType::Elite)
    {
        _CurrentMonster = std::make_unique<EliteMonster>(data);
    }
    else  // Normal
    {
        _CurrentMonster = std::make_unique<NormalMonster>(data);
    }

    // 6. 전투 상태 초기화
    _BattleType = type;
    _IsBattleActive = true;
    _Result = BattleResult{};

    _CurrentRound = 0;
    _ItemReservations.clear();

    // 전투 시작 시 파티원 어그로 초기화
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();
    for (const auto& member : party)
    {
        if (member)
            member->ResetAggro();
    }

    return true;
}

void BattleManager::EndBattle()
{
    if (!_IsBattleActive)
        return;

    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    // 1. 전투 종료 시 숙련도 정산
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            member->ProcessBattleEndProficiency();
        }
    }

    // 2. 모든 파티원 버프 초기화
    for (const auto& member : party)
    {
        if (member)
            member->ResetTempStats();
    }

    // 3. 승리 시 보상 계산
    if (_Result.Victory)
    {
        Player* mainPlayer = gm->GetMainPlayer().get();
        CalculateReward(mainPlayer, _CurrentMonster.get());
    }

    // 4. 사망한 동료 제거
    gm->RemoveDeadCompanions();

    // 5. 예약 아이템 정리
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
    if (!_IsBattleActive || !_CurrentMonster)
        return false;

    GameManager* gm = GameManager::GetInstance();

    if (_IsPlayerTurn)
    {
        SetCurrentRound(_CurrentRound + 1);
        // 🧑 플레이어 턴
        ProcessTurn(_CurrentMonster.get());

        if (_CurrentMonster->IsDead())
        {
            _Result.Victory = true;
            _Result.IsCompleted = true;
            return false;
        }

        _IsPlayerTurn = false;   // ⭐ 다음은 몬스터
        return true;             // ⭐ 여기서 끊는다
    }
    else
    {
        // 👹 몬스터 턴
        Player* target = SelectMonsterTarget();
        ProcessAttack(_CurrentMonster.get(), target);

        if (gm->GetMainPlayer()->IsDead())
        {
            _Result.Victory = false;
            _Result.IsCompleted = true;
            return false;
        }
        // ⭐ 라운드 종료 처리 (여기가 정답)
        const auto& party = gm->GetParty();
        for (const auto& member : party)
        {
            if (member && !member->IsDead())
            {
                member->ProcessRoundEnd();
                member->ProcessSkillCooldowns();
            }
        }

        _IsPlayerTurn = true;    // ⭐ 다시 플레이어
        return true;
    }
}

// ========================================
// ===== 아이템 예약 시스템 =====
// ========================================

bool BattleManager::ReserveItemUse(Player* player, int slotIndex)
{
    if (!player) {
        //PrintManager::GetInstance()->PrintLogLine("플레이어가 유효하지 않습니다.", ELogImportance::WARNING);
        PushLog("플레이어가 유효하지 않습니다.", EBattleLogType::Important);
        return false;
    }

    // 인벤토리 확인
    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory)) {
        //PrintManager::GetInstance()->PrintLogLine("인벤토리가 없습니다.", ELogImportance::WARNING);
        PushLog("인벤토리가 없습니다.", EBattleLogType::Important);
        return false;
    }

    // 슬롯 유효성 검증
    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item) {
        //PrintManager::GetInstance()->PrintLogLine("해당 슬롯에 아이템이 없습니다.", ELogImportance::WARNING);
        PushLog("해당 슬롯에 아이템이 없습니다.", EBattleLogType::Important);
        return false;
    }

    // 이미 예약되어 있는지 확인
    if (item->IsReserved()) {
        /*PrintManager::GetInstance()->PrintLogLine(
            item->GetName() + "은(는) 이미 예약되어 있습니다.",
            ELogImportance::WARNING
        );*/
        PushLog(item->GetName() + "은(는) 이미 예약되어 있습니다.", EBattleLogType::Important);
        return false;
    }

    // 예약 등록
    item->Reserve(_CurrentRound);
    _ItemReservations.push_back({ slotIndex, player, true });

    /*PrintManager::GetInstance()->PrintLogLine(
        item->GetName() + " 사용 예약 완료! (조건: " +
        item->GetUseConditionDescription() + ")",
        ELogImportance::DISPLAY
    );*/
    PushLog(item->GetName() + " 사용 예약 완료! (조건: " +
        item->GetUseConditionDescription() + ")", EBattleLogType::Important);

    return true;
}

bool BattleManager::CancelItemReservation(Player* player, int slotIndex)
{
    if (!player) return false;

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory)) return false;

    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item || !item->IsReserved()) {
        /*PrintManager::GetInstance()->PrintLogLine(
            "해당 슬롯에 예약된 아이템이 없습니다.",
            ELogImportance::WARNING
        );*/
        PushLog("해당 슬롯에 예약된 아이템이 없습니다.",EBattleLogType::Important);
        return false;
    }

    // 예약 목록에서 제거
    for (auto& reservation : _ItemReservations) {
        if (reservation.User == player &&
            reservation.SlotIndex == slotIndex &&
            reservation.IsActive) {

            reservation.IsActive = false;
            item->CancelReservation();

            /*PrintManager::GetInstance()->PrintLogLine(
                item->GetName() + " 예약이 취소되었습니다.",
                ELogImportance::DISPLAY
            );*/
            PushLog(item->GetName() + " 예약이 취소되었습니다.", EBattleLogType::Important);
            return true;
        }
    }

    return false;
}

// ===== 특정 플레이어의 예약 아이템 체크 및 사용 =====
bool BattleManager::TryUseReservedItem(Player* player)
{
    if (!player) return false;

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory)) return false;

    // 해당 플레이어의 예약만 찾기
    for (auto& reservation : _ItemReservations)
    {
        if (!reservation.IsActive || reservation.User != player)
            continue;

        // 아이템 가져오기
        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (!item)
        {
            // 아이템이 사라짐 → 예약 취소
            /*PrintManager::GetInstance()->PrintLogLine(
                player->GetName() + "의 예약 아이템(슬롯 " + std::to_string(reservation.SlotIndex) + ")이 사라졌습니다.",
                ELogImportance::WARNING
            );*/
            PushLog(player->GetName() + "의 예약 아이템(슬롯 " + std::to_string(reservation.SlotIndex) + ")이 사라졌습니다.", EBattleLogType::Important);
            reservation.IsActive = false;
            continue;
        }

        // ===== 조건 체크 (IItem::CanUse) =====
        if (!item->CanUse(*player, _CurrentRound))
        {
            // 조건 불만족 → 예약 유지, 일반 공격 진행
  // TODO: BattleScene에서 "조건 미달, 공격 진행" 로그 표시
            return false;
        }

        // ===== 조건 만족 → 자동 사용 =====
        /*PrintManager::GetInstance()->PrintLogLine(
            ">>> " + player->GetName() + "의 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)",
            ELogImportance::DISPLAY
        );*/
        PushLog(">>> " + player->GetName() + "의 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)", EBattleLogType::Important);

        // 효과 적용
        item->ApplyEffect(*player);

        // 인벤토리에서 제거
        inventory->RemoveItem(reservation.SlotIndex, 1);

        // 예약 취소
        item->CancelReservation();
        reservation.IsActive = false;

        return true;  // 아이템 사용으로 턴 소모
    }

    return false;  // 예약 없음 또는 조건 불만족
}

bool BattleManager::ProcessReservedItems()
{
    if (_ItemReservations.empty()) return false;

    // TODO: BattleScene에서 "=== 예약된 아이템 처리 중 ===" 로그 표시

    bool anyItemUsed = false;

    // 활성화된 예약만 처리
    for (auto& reservation : _ItemReservations)
    {
        if (!reservation.IsActive) continue;

        Player* user = reservation.User;
        Inventory* inventory = nullptr;

        if (!user->TryGetInventory(inventory)) continue;

        // 아이템 가져오기
        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (!item)
        {
            // 아이템이 사라짐 → 예약 취소
            /*PrintManager::GetInstance()->PrintLogLine(
                "슬롯 [" + std::to_string(reservation.SlotIndex) + "]의 아이템이 없어져 예약이 취소되었습니다.",
                ELogImportance::WARNING
            );*/
            PushLog("슬롯 [" + std::to_string(reservation.SlotIndex) + "]의 아이템이 없어져 예약이 취소되었습니다.", EBattleLogType::Important);
            reservation.IsActive = false;
            continue;
        }

        // ===== 조건 체크 (IItem::CanUse) =====
        if (!item->CanUse(*user, _CurrentRound))
        {
            // 조건 불만족 → 예약 유지 (다음 턴 재시도)
             // TODO: BattleScene에서 "조건 미달, 대기 중" 로그 표시
            continue;
        }

        // ===== 조건 만족 → 자동 사용 =====
        /*PrintManager::GetInstance()->PrintLogLine(
            ">>> " + user->GetName() + "의 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)",
            ELogImportance::DISPLAY
        );*/
        PushLog(">>> " + user->GetName() + "의 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)", EBattleLogType::Important);

        // 효과 적용
        item->ApplyEffect(*user);

        // 인벤토리에서 제거
        inventory->RemoveItem(reservation.SlotIndex, 1);

        // 예약 취소
        item->CancelReservation();
        reservation.IsActive = false;

        anyItemUsed = true;
    }

    // 비활성화된 예약 정리
    _ItemReservations.erase(
        std::remove_if(_ItemReservations.begin(), _ItemReservations.end(),
            [](const ItemReservation& r) { return !r.IsActive; }),
        _ItemReservations.end()
    );

    return anyItemUsed;
}

// 직업 우선순위 반환 함수 (전역 함수)
// Archer(0) > Priest(1) > Warrior(2) > Mage(3)
int GetJobPriority(ICharacter* character)
{
    if (dynamic_cast<Archer*>(character))  return 0;
    if (dynamic_cast<Priest*>(character))  return 1;
    if (dynamic_cast<Warrior*>(character)) return 2;
    if (dynamic_cast<Mage*>(character))    return 3;

    return 99; // 예외 / 알 수 없는 타입
}

void BattleManager::PushLog(const std::string& msg, EBattleLogType type)
{
    _BattleLogs.push_back({ msg, type });
}

std::vector<BattleLog> BattleManager::ConsumeLogs()
{
    std::vector<BattleLog> result = std::move(_BattleLogs);
    _BattleLogs.clear();
    return result;
}

//RequestFlush();