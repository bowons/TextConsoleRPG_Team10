#include "../../include/Manager/BattleManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Unit/NormalMonster.h"
#include "../../include/Unit/EliteMonster.h"
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
#include "../../include/Skill/ISkill.h"
#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/SoundPlayer.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Data/FloorScalingData.h"
#include "../../include/UI/IBattleAnimationCallback.h"
#include <iostream>
#include <tuple>
#include <memory>
#include <algorithm>
#include <Windows.h>
#include <optional>
#include "../../include/UI/Scenes/BattleScene.h"

// ===== 직업 우선순위 반환 함수 (전역 함수) =====
// Archer(0) > Priest(1) > Warrior(2) > Mage(3)
int GetJobPriority(ICharacter* character)
{
    if (dynamic_cast<Archer*>(character))  return 0;
    if (dynamic_cast<Priest*>(character))  return 1;
    if (dynamic_cast<Warrior*>(character)) return 2;
    if (dynamic_cast<Mage*>(character))    return 3;

    return 99; // 예외 / 알 수 없는 타입
}

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

        // 1순위: 어그로가 더 높은 대상
        if (aggro > maxAggro)
        {
            maxAggro = aggro;
            target = member.get();
        }
        // 2순위: 어그로가 동일하면 민첩성이 낮은 대상 (약한 대상 우선)
        else if (aggro == maxAggro && target != nullptr)
        {
            if (member->GetDex() < target->GetDex())
            {
                target = member.get();
            }
        }
    }

    // 어그로 동점이거나 타겟 없으면 메인 플레이어
    return target ? target : gm->GetMainPlayer().get();
}

// ===== 우선순위 정렬된 파티원 목록 갱신 =====
void BattleManager::RefreshTurnOrder()
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    _TurnOrder.clear();

    // 생존한 파티원만 수집
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            _TurnOrder.push_back(member.get());
        }
    }

    // 우선순위 정렬 (1순위: 직업, 2순위: DEX)
    std::sort(_TurnOrder.begin(), _TurnOrder.end(),
        [](Player* a, Player* b)
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

    // 인덱스 초기화 (순환 시작)
    if (_CurrentPartyMemberIndex >= static_cast<int>(_TurnOrder.size()))
    {
        _CurrentPartyMemberIndex = 0;
    }
}

void BattleManager::ProcessTurn(ICharacter* Def)
{
    // ===== 1턴 = 1명만 행동 =====
    if (_TurnOrder.empty())
    {
        RefreshTurnOrder();
    }

    if (_TurnOrder.empty())
    {
        PushLog("[경고] 생존한 파티원이 없습니다.", EBattleLogType::Important);
        return;
    }

    // ===== 현재 턴의 파티원 가져오기 =====
    Player* currentPlayer = _TurnOrder[_CurrentPartyMemberIndex];

    //PushLog("=== " + currentPlayer->GetName() + "의 턴 ===", EBattleLogType::Important);

    // 1️⃣ 예약 아이템 체크
    if (TryUseReservedItem(currentPlayer))
    {
        PushLog(currentPlayer->GetName() + "은(는) 아이템 사용으로 턴 종료", EBattleLogType::Important);
        RequestFlush(EBattleFlushType::PlayerItem);
        // ⭐ 아이템 사용 후 공격하지 않고 턴 종료
    }
    else
    {
        // 2️⃣ 공격 진행
        ProcessAttack(currentPlayer, Def);
    }

    // ===== 다음 파티원으로 순환 =====
    _CurrentPartyMemberIndex++;
    if (_CurrentPartyMemberIndex >= static_cast<int>(_TurnOrder.size()))
    {
        _CurrentPartyMemberIndex = 0;  // 사이클 완료 → 다시 Archer부터
        RefreshTurnOrder();  // 사망자 제외하고 재정렬
    }
}

void BattleManager::ProcessAttack(ICharacter* Atk, ICharacter* Def)
{
    if (!Atk || !Def)
        return;

    // Player의 스킬 우선 사용
    if (Player* player = dynamic_cast<Player*>(Atk))
    {
        int skillIndex = player->SelectBestSkill(Def);
        if (skillIndex >= 0)
        {
            SkillResult result = player->UseSkill(skillIndex, Def);
            if (result.Success)
            {
                player->TrackMPSpent(player->GetSkills()[skillIndex]->GetMPCost());
                PushLog(player->GetName() + "의 " + result.SkillName + "!", EBattleLogType::Important);

                ESkillType skillType = player->GetSkills()[skillIndex]->GetType();

                // 버프/힐 스킬은 데미지 처리 스킵
                if (skillType == ESkillType::Buff || skillType == ESkillType::Heal)
                {
                    if (!result.Message.empty())
                    {
                        PushLog(result.Message, EBattleLogType::Important);
                        RequestFlush(EBattleFlushType::PlayerSkill);
                    }
                    return;
                }

                // 다단 히트 처리
                if (result.HitCount > 1)
                {
                    int totalDamage = 0;
                    for (int i = 0; i < result.HitCount; ++i)
                    {
                        int damage = Def->TakeDamage(player, result.Value);
                        totalDamage += damage;
                        player->ModifyAggro(10);

                        PushLog("타격 " + std::to_string(i + 1) + "/" + std::to_string(result.HitCount) +
                            ": " + std::to_string(damage) + " 데미지!", EBattleLogType::Important);

                        if (Def->IsDead())
                            break;
                    }
                    PushLog("총 데미지: " + std::to_string(totalDamage), EBattleLogType::Important);
                }
                else
                {
                    int damage = Def->TakeDamage(player, result.Value);
                    player->ModifyAggro(10);
                    PushLog(Def->GetName() + "에게 " + std::to_string(damage) + " 데미지!", EBattleLogType::Important);
                }

                if (!result.Message.empty())
                {
                    PushLog(result.Message, EBattleLogType::Important);
                }
                RequestFlush(EBattleFlushType::PlayerAttack);
                return;  // 스킬 사용 성공 → 일반 공격 스킵
            }
        }
    }

    // 일반 공격
    std::tuple<std::string, int> attackResult = Atk->Attack(Def);
    std::string attackType = std::get<0>(attackResult);
    int baseDamage = std::get<1>(attackResult);

    // Boss 특수 스킬 처리
    if (Boss* boss = dynamic_cast<Boss*>(Atk))
    {
        if (attackType == "어둠의 폭풍")
        {
            ProcessAOEAttack(attackType, baseDamage, boss);
            RequestFlush(EBattleFlushType::BossAttack);
            return;
        }

        if (attackType == "공포의 속삭임")
        {
            ProcessDebuff(attackType, boss);
            RequestFlush(EBattleFlushType::BossAttack);
            return;
        }
    }

    // 실제 데미지 적용
    int Damage = 0;

    // Player's Attack() already calls TakeDamage internally
    // If it's a Player, use the damage from Attack()'s return value
    if (dynamic_cast<Player*>(Atk))
    {
        Damage = baseDamage;
    }
    // For Monsters, explicitly call TakeDamage
    else if (IMonster* monster = dynamic_cast<IMonster*>(Atk))
    {
        Damage = Def->TakeDamage(Atk, baseDamage);
    }

    // 몬스터 공격 시 피격자 어그로 감소
    if (IMonster* monster = dynamic_cast<IMonster*>(Atk))
    {
        if (Player* targetPlayer = dynamic_cast<Player*>(Def))
        {
            bool isCritical = (attackType.find("치명타") != std::string::npos ||
                attackType.find("!") != std::string::npos);

            targetPlayer->ModifyAggro(isCritical ? -20 : -10);
        }
    }

    // 공격 로그 출력
    bool isCritical = (attackType == "치명타!");
    bool isSpecialSkill = (attackType != "공격" && attackType != "치명타!");

    if (isCritical)
    {
        PushLog("💥 " + Atk->GetName() + "의 " + attackType + " 💥", EBattleLogType::Important);
    }
    else if (isSpecialSkill)
    {
        PushLog("⚡ " + Atk->GetName() + "의 " + attackType + "! ⚡", EBattleLogType::Important);
    }
    else
    {
        PushLog(Atk->GetName() + "의 " + attackType, EBattleLogType::Important);
    }

    PushLog(Def->GetName() + "에게 " + std::to_string(Damage) + " 데미지!", EBattleLogType::Important);

    RequestFlush(EBattleFlushType::MonsterAttack);
}

// ===== 광역 공격 처리 (Boss 전용) =====
void BattleManager::ProcessAOEAttack(const std::string& skillName, int damage, ICharacter* attacker)
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    PushLog("🌪️ " + attacker->GetName() + "의 " + skillName + "! 🌪️", EBattleLogType::Important);

    // 파티 전체에 데미지
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            int actualDamage = member->TakeDamage(attacker, damage);

            // 광역 공격 피격 시 어그로 -10
            member->ModifyAggro(-10);

            PushLog("  → " + member->GetName() + "에게 " + std::to_string(actualDamage) + " 데미지!", EBattleLogType::Important);
        }
    }
    RequestFlush(EBattleFlushType::BossAttack);
}

// ===== 디버프 처리 (Boss 전용) =====
void BattleManager::ProcessDebuff(const std::string& skillName, ICharacter* attacker)
{
    GameManager* gm = GameManager::GetInstance();
    const auto& party = gm->GetParty();

    PushLog("😱 " + attacker->GetName() + "의 " + skillName + "! 😱", EBattleLogType::Important);

    // 파티 전체에 공격력 디버프 (-30%, 2라운드)
    for (const auto& member : party)
    {
        if (member && !member->IsDead())
        {
            int debuffAmount = -static_cast<int>(member->GetAtk() * 0.3f);
            member->ApplyTempAtkBuff(debuffAmount, 2);

            PushLog("  → " + member->GetName() + "의 공격력이 감소했다! (" + std::to_string(debuffAmount) + ", 2라운드)", EBattleLogType::Important);
        }
    }
    RequestFlush(EBattleFlushType::BossDebuff);
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
            PushLog("파티원 모두 ", EBattleLogType::Important);
        }
        else
        {
            PushLog(mainPlayer->GetName() + "은(는) ", EBattleLogType::Important);
        }
        PushLog(std::to_string(Exp) + "의 경험치를 획득했습니다.", EBattleLogType::Important);

        // 파티 전체에 경험치 분배
        for (const auto& member : party)
        {
            if (member)
            {
                member->GainExp(Exp);
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
        PushLog(mainPlayer->GetName() + "은(는) " + std::to_string(Gold) + "G를 획득했습니다.", EBattleLogType::Important);
        mainPlayer->GainGold(Gold);
        PushLog(mainPlayer->GetName() + "의 소지 골드량은 " + std::to_string(mainPlayer->GetGold()) + " G" + "입니다.", EBattleLogType::Important);
    }

    // ===== 아이템: 메인 플레이어 인벤토리에만 추가 =====
    if (DroppedItem && mainPlayer)
    {
        std::string ItemName = DroppedItem->GetName();

        // 메인 플레이어 인벤토리 접근
        Inventory* inventory = nullptr;
        if (!mainPlayer->TryGetInventory(inventory))
        {
            PushLog(mainPlayer->GetName() + "은(는) 인벤토리가 없어 아이템을 얻지 못했습니다.", EBattleLogType::Important);
            PushLog("", EBattleLogType::Important);
            return;
        }

        int Remain;
        if (inventory->AddItem(std::move(DroppedItem), 1, Remain))
        {
            PushLog(mainPlayer->GetName() + "은(는) " + ItemName + "을 보상으로 얻었습니다.", EBattleLogType::Important);
        }
        else
        {
            PushLog(mainPlayer->GetName() + "은(는) 인벤토리가 가득 차 있어 아이템을 얻지 못했습니다.", EBattleLogType::Important);
        }
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

        Player* owner = reservation.Owner;
        if (!owner)
            continue;

        Inventory* inventory = nullptr;
        if (!owner->TryGetInventory(inventory))
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

// ===== 전투 상태 완전 리셋 =====
void BattleManager::ResetAll()
{
    // 1. 활성 전투가 있으면 종료
    if (_IsBattleActive)
    {
        EndBattle();
    }

    // 2. 모든 내부 상태 초기화
    _CurrentMonster.reset();
    _BattleType = EBattleType::None;
    _IsBattleActive = false;
    _Result = BattleResult{};

    // 3. 턴 시스템 초기화
    _CurrentRound = 0;
    _CurrentPartyMemberIndex = 0;
    _TurnOrder.clear();
    _IsPlayerTurn = true;

    // 4. 아이템 예약 정리
    _ItemReservations.clear();

    // 5. 로그 정리
    _BattleLogs.clear();

    // 6. 애니메이션 콜백 해제
    _AnimationCallback = nullptr;

    // 7. 플러시 콜백 해제
    _flushCallback = nullptr;
}

bool BattleManager::ProcessBattleTurn()
{
    // 한 턴 처리 함수, 플레이어 > 몬스터 순서로 진행, 배틀 종료 시 false 반환
    // 1. 전투 중이 아니거나 몬스터가 없으면 false 반환
    if (!_IsBattleActive || !_CurrentMonster)
        return false;

    // ⚠️ 몬스터가 이미 죽었는지 체크 (중복 호출 방지)
    if (_CurrentMonster->IsDead())
    {
        _Result.Victory = true;
        _Result.IsCompleted = true;
        return false;
    }

    if (_IsPlayerTurn)
    {
        // ⭐ 플레이어 턴 시작 시 라운드 증가 (한 번만!)
        SetCurrentRound(_CurrentRound + 1);

        // 4. 플레이어 턴: ProcessTurn(Monster)
        ProcessTurn(_CurrentMonster.get());

        // 5. 몬스터 사망 확인
        if (_CurrentMonster->IsDead())
        {
            _Result.Victory = true;
            _Result.IsCompleted = true;
            SoundPlayer::GetInstance()->PlayMonsterSFX(_CurrentMonster.get()->GetName(), "Dead");
            return false;
        }
        _IsPlayerTurn = false;   // ⭐ 다음은 몬스터
        return true;             // ⭐ 여기서 끊는다
    }
    else
    {
        // 6. 몬스터 턴: 타겟 선정 후 공격
        Player* target = SelectMonsterTarget();
        GameManager* gm = GameManager::GetInstance();
        ProcessAttack(_CurrentMonster.get(), target);

        // 7. 메인 플레이어 사망 확인 (게임 오버 조건)
        if (gm->GetMainPlayer()->IsDead())
        {
            _Result.Victory = false;
            _Result.IsCompleted = true;

            SoundPlayer::GetInstance()->PlaySFX("Player_Dead");
            PushLog("용사의 여정이 끝났습니다... 전투에서 패배했습니다.", EBattleLogType::Important);
            return false;
        }

        // 8. 라운드 종료 처리: 파티 전체 버프 감소 + 스킬 쿨타임 감소
        const auto& party = gm->GetParty();
        for (const auto& member : party)
        {
            if (member && !member->IsDead())
            {
                member->ProcessRoundEnd();  // 버프 라운드 감소
            }
        }

        _IsPlayerTurn = true;    // ⭐ 다시 플레이어
        return true;
    }
}


// ========================================
// ===== 아이템 예약 시스템 =====
// ========================================

bool BattleManager::ReserveItemUse(Player* owner, Player* target, int slotIndex)
{
    if (!owner || !target)
    {
        PushLog("플레이어가 유효하지 않습니다.", EBattleLogType::Important);
        return false;
    }

    // 인벤토리 확인 (소유자 = 메인 플레이어)
    Inventory* inventory = nullptr;
    if (!owner->TryGetInventory(inventory))
    {
        PushLog("인벤토리가 없습니다.", EBattleLogType::Important);
        return false;
    }

    // 슬롯 유효성 검증
    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item)
    {
        PushLog("해당 슬롯에 아이템이 없습니다.", EBattleLogType::Important);
        return false;
    }

    // 이미 예약되어 있는지 확인
    if (item->IsReserved())
    {
        PushLog(item->GetName() + "은(는) 이미 예약되어 있습니다.", EBattleLogType::Important);
        return false;
    }

    // 예약 등록
    item->Reserve(_CurrentRound);
    _ItemReservations.push_back({ slotIndex, owner, target, true });

    PushLog(item->GetName() + " 사용 예약 완료! (대상: " + target->GetName() +
        ", 조건: " + item->GetUseConditionDescription() + ")", EBattleLogType::Important);

    return true;
}

bool BattleManager::CancelItemReservation(Player* owner, int slotIndex)
{
    if (!owner) return false;

    Inventory* inventory = nullptr;
    if (!owner->TryGetInventory(inventory)) return false;

    IItem* item = inventory->GetItemAtSlot(slotIndex);
    if (!item || !item->IsReserved())
    {
        PushLog("해당 슬롯에 예약된 아이템이 없습니다.", EBattleLogType::Important);
        return false;
    }

    // 예약 목록에서 제거
    for (auto& reservation : _ItemReservations)
    {
        if (reservation.Owner == owner &&
            reservation.SlotIndex == slotIndex &&
            reservation.IsActive)
        {
            reservation.IsActive = false;
            item->CancelReservation();

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

    // 해당 플레이어가 "대상"인 예약 찾기
    for (auto& reservation : _ItemReservations)
    {
        if (!reservation.IsActive || reservation.Target != player)
            continue;

        // 소유자의 인벤토리에서 아이템 가져오기
        Inventory* inventory = nullptr;
        if (!reservation.Owner->TryGetInventory(inventory) || !inventory)
        {
            PushLog(reservation.Owner->GetName() + "의 인벤토리가 없습니다.", EBattleLogType::Important);
            reservation.IsActive = false;
            continue;
        }

        // 아이템 가져오기
        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (!item)
        {
            // 아이템이 사라짐 → 예약 취소
            PushLog(player->GetName() + "의 예약 아이템(슬롯 " + std::to_string(reservation.SlotIndex) + ")이 사라졌습니다.", EBattleLogType::Important);
            reservation.IsActive = false;
            continue;
        }

        // ===== 조건 체크 (IItem::CanUse) =====
        // **중요: player는 "대상"이므로 조건 체크 시 사용**
        if (!item->CanUse(*player, _CurrentRound))
        {
            // 조건 불만족 → 예약 유지, 일반 공격 진행
            return false;
        }

        // ===== 조건 만족 → 자동 사용 =====
        PushLog(">>> " + player->GetName() + "에게 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)", EBattleLogType::Important);

        // **효과 적용 - player(대상)에게 적용**
        item->ApplyEffect(*player);

        // 인벤토리에서 제거 (소유자의 인벤토리)
        inventory->RemoveItem(reservation.SlotIndex, 1);

        // 예약 취소
        item->CancelReservation();
        reservation.IsActive = false;

        SoundPlayer::GetInstance()->PlaySFX("Item_Weapon");
        return true;  // 아이템 사용으로 턴 소모
    }

    return false;  // 예약 없음 또는 조건 불만족
}

bool BattleManager::ProcessReservedItems()
{
    if (_ItemReservations.empty()) return false;

    bool anyItemUsed = false;

    // 활성화된 예약만 처리
    for (auto& reservation : _ItemReservations)
    {
        if (!reservation.IsActive) continue;

        Player* owner = reservation.Owner;
        Player* target = reservation.Target;
        Inventory* inventory = nullptr;

        if (!owner->TryGetInventory(inventory)) continue;

        // 아이템 가져오기
        IItem* item = inventory->GetItemAtSlot(reservation.SlotIndex);
        if (!item)
        {
            // 아이템이 사라짐 → 예약 취소
            PushLog("슬롯 [" + std::to_string(reservation.SlotIndex) + "]의 아이템이 없어져 예약이 취소되었습니다.", EBattleLogType::Important);
            reservation.IsActive = false;
            continue;
        }

        // ===== 조건 체크 (IItem::CanUse) - 대상 기준 =====
        if (!item->CanUse(*target, _CurrentRound))
        {
            // 조건 불만족 → 예약 유지 (다음 턴 재시도)
            continue;
        }

        // ===== 조건 만족 → 자동 사용 =====
        PushLog(">>> " + target->GetName() + "에게 " + item->GetName() + " 자동 사용! (" +
            item->GetUseConditionDescription() + " 만족)", EBattleLogType::Important);

        // 효과 적용 - 대상에게
        item->ApplyEffect(*target);

        // 인벤토리에서 제거 - 소유자의 인벤토리
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
//플러시 사용시 호출
//RequestFlush(type);