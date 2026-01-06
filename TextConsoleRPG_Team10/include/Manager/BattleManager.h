#pragma once
#include "../Singleton.h"
#include "../Unit/IMonster.h"
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <functional>

class Player;
class ICharacter;

// ===== 전역 함수 선언 =====
// 직업 우선순위 반환 (Archer=0, Priest=1, Warrior=2, Mage=3)
int GetJobPriority(ICharacter* character);

// ===== 전투 결과 구조체 =====
// Scene에서 전투 종료 후 결과를 조회하기 위한 데이터
struct BattleResult
{
    bool Victory = false;  // 승리 여부
    bool IsCompleted = false;       // 전투 종료 여부
    int ExpGained = 0;     // 획득 경험치
    int GoldGained = 0;         // 획득 골드
    std::string ItemName = "";      // 획득 아이템 이름 (없으면 빈 문자열)
};

// ===== 전투 타입 =====
enum class EBattleType
{
    None,
    Normal,     // 일반 몬스터
    Elite,      // 정예 몬스터
    Boss        // 보스
};
// ===== 전투 로그 =====
enum class EBattleLogType
{
    Normal,
    Warning,
    Important
};
struct BattleLog
{
    std::string Message;
    EBattleLogType Type;
};

// ===== 아이템 예약 구조체 =====
struct ItemReservation
{
    int SlotIndex;      // 인벤토리 슬롯 인덱스
    Player* User; // 사용자
    bool IsActive; // 예약 활성화 여부
};

class BattleManager : public Singleton<BattleManager>
{
private:
    // 현재 전투 상태
    std::unique_ptr<IMonster> _CurrentMonster;
    EBattleType _BattleType = EBattleType::None;
    bool _IsBattleActive = false;
    BattleResult _Result;

    // ===== 아이템 예약 시스템 =====
    int _CurrentRound = 0;  // 현재 라운드 (0부터 시작)
    std::vector<ItemReservation> _ItemReservations;  // 예약 목록

public:
    // ===== Scene 친화적 인터페이스 =====

    // 전투 시작 (몬스터 생성만 담당)
    // GameManager에서 메인 플레이어 레벨 가져옴
    // Normal,Elite: DataManager에서 랜덤 몬스터 생성
    // Boss: 보스 몬스터 생성
    // type: 전투 타입 (Normal?Elite/Boss)
    // return: 성공 시 true, 실패 시 false
    bool StartBattle(EBattleType type, int Floor);

    // 전투 종료 처리
    // 승리 시: 보상 처리 (CalculateReward 호출)
    // 패배 시: 보상 없음
    // 모든 파티원 버프 초기화
    // 사망한 동료 제거 (GameManager::RemoveDeadCompanions)
    // 전투 상태 초기화
    void EndBattle();

    // 1턴 실행 (Scene의 Update에서 호출)
    // 플레이어 턴: ProcessTurn(Player, Monster)
    // 몬스터 사망 체크 → 승리 처리
    // 몬스터 턴: ProcessAttack(Monster, Player)
    // 플레이어 사망 체크 → 패배 처리
    // 라운드 종료: 파티 전체 ProcessRoundEnd()
    // return: 전투 계속 시 true, 종료 시 false
    bool ProcessBattleTurn();

    // ===== 전투 상태 조회 =====

    // 전투 진행 중 여부
    inline bool IsBattleActive() const { return _IsBattleActive; }

    // 현재 전투 타입
    inline EBattleType GetBattleType() const { return _BattleType; }

    // 현재 전투 중인 몬스터 (nullptr 가능)
    inline IMonster* GetCurrentMonster() const { return _CurrentMonster.get(); }

    // 전투 결과 (전투 종료 후 유효)
    inline const BattleResult& GetBattleResult() const { return _Result; }

    // 현재 라운드 반환
    inline int GetCurrentRound() const { return _CurrentRound; }
    inline void SetCurrentRound(int round) { _CurrentRound = round; }

    // ===== 아이템 예약 시스템 인터페이스 =====

    // 아이템 사용 예약
    // player: 사용자
    // slotIndex: 예약할 슬롯 인덱스
    // return: 예약 성공 시 true
    bool ReserveItemUse(Player* player, int slotIndex);

    // 아이템 예약 취소
    // player: 사용자
    // slotIndex: 취소할 슬롯
    // return: 취소 성공 시 true
    bool CancelItemReservation(Player* player, int slotIndex);

    // 현재 예약된 아이템 목록 조회 (UI 표시용)
    // return: 활성화된 예약 목록
    const std::vector<ItemReservation>& GetActiveReservations() const { return _ItemReservations; }

    // 예약된 아이템 처리 (내부 호출 - ProcessBattleTurn에서 자동 호출됨)
    // 각 예약에 대해 IItem::CanUse 체크
    // 조건 만족 시 IItem::ApplyEffect 호출
    // 사용 실패 시 예약 유지 (다음 턴 재시도)
    bool ProcessReservedItems();

    // ===== 내부 로직 =====

    // 특정 플레이어의 예약 아이템 체크 및 사용 시도
    // player: 대상 플레이어
    // return: 아이템 사용 시 true (턴 소모), 아니면 false
    bool TryUseReservedItem(Player* player);

    // 몬스터 타겟 선정 (어그로 최댓값 기준)
    // return: 가장 어그로가 높은 생존 파티원 (없으면 메인 플레이어)
    Player* SelectMonsterTarget();

    // 턴 처리 (아이템 사용 + 공격)
    // Player: 체력/공격력 포션 자동 사용 판단 → ProcessAttack 호출
    // Monster: 즉시 ProcessAttack 호출
    void ProcessTurn(ICharacter* Def);

    // 공격 처리 (피해 계산 + 로그 출력)
    // Attack() 호출
    // 피해량 계산
    // PrintManager로 전투 로그 출력
    void ProcessAttack(ICharacter* Atk, ICharacter* Def);

    // 광역 공격 처리 (Boss 페이즈 2용)
    // skillName: 스킬명 (예: "어둠의 폭풍")
 // damage: 기본 데미지
    // attacker: 공격자
    void ProcessAOEAttack(const std::string& skillName, int damage, ICharacter* attacker);

    // 디버프 처리 (Boss 페이즈 2용)
    // skillName: 스킬명
    // attacker: 공격자
    void ProcessDebuff(const std::string& skillName, ICharacter* attacker);

    // 보상 계산 및 분배 (내부 호출용)
    // 경험치: 파티 전체 동일 수치 획득
    // 골드: 메인 플레이어만 획득
    // 아이템: 메인 플레이어 인벤토리에 추가
    // BattleResult 업데이트
    void CalculateReward(Player* P, IMonster* M);
    // ===== 전투 로그 시스템 =====
private:
    std::vector<BattleLog> _BattleLogs;

public:
    void PushLog(const std::string& msg, EBattleLogType type = EBattleLogType::Normal);
    std::vector<BattleLog> ConsumeLogs(); // BattleScene에서 가져감

    // ===== 씬 콜백 =====
    public:
        using FlushCallback = std::function<void()>;

        void SetFlushCallback(FlushCallback cb)
        {
            _flushCallback = cb;
        }

        void RequestFlush()
        {
            if (_flushCallback)
                _flushCallback();
        }

private:
    FlushCallback _flushCallback;
    bool _IsPlayerTurn = true;
};
