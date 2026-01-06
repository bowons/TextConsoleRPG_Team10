#pragma once
#include "../UIScene.h"
#include "../IBattleAnimationCallback.h"
#include <vector>
#include <string>
#include <Windows.h>
#include <fstream>
#include <nlohmann/json.hpp>

// ===== 전투 입력 상태 =====
enum class EBattleInputState
{
    Playing,        // 전투 진행 중 (턴 처리)
    ResultShown,    // 전투 종료, 결과 로그 표시 중
    EndWaiting      // 결과 확인 완료, 씬 이동 대기
};

// 애니메이션 구조체
struct ConsoleAnimation
{
    std::vector<std::string> Frames;
    std::vector<int> TimestampsMs;
};

//ConsoleAnimation _CurrentAnimation;
//size_t _CurrentAnimFrame = 0;
//float _AnimElapsedTime = 0.0f;
extern unsigned __int64 _CurrentAnimFrame;
extern float _AnimElapsedTime;
extern ConsoleAnimation _CurrentAnimation;

// 전투 Scene
class BattleScene : public UIScene, public IBattleAnimationCallback
{
private:
    // ===== 전투 상태 =====
    int _CurrentTurn;      // 현재 턴 수
    bool _PlayerTurn;      // true: 플레이어 턴, false: 적 턴
    int _SelectedAction;   // 선택한 행동 (0: 공격, 1: 스킬, 2: 아이템)
    int _SelectedTarget;   // 선택한 대상 인덱스
    bool _BattleEnd;       // 전투 종료 여부

    // ===== 시스템 로그 =====
    std::vector<std::string> _SystemLogs;

    // ===== 인벤토리 UI =====
    int _SelectedItemSlot;  // 선택된 인벤토리 슬롯 (0~4)
    bool _IsSelectingItem;  // 아이템 선택 모드 활성화 여부
    int _SelectedPartyIndex; // 선택된 파티원 인덱스 (0~3)
    bool _IsCancelMode;    // 예약 취소 모드 여부

    // ===== 애니메이션 (간소화) =====
    bool _IsWaitingForAnimation;
    float _AnimationWaitTimer;

    EBattleInputState _InputState = EBattleInputState::Playing;

public:
    BattleScene();
    ~BattleScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
    
    // ===== IBattleAnimationCallback 구현 (간소화) =====
    void SetPanelAnimation(const std::string& panelName,
     const std::string& animJsonFile, float duration) override;
    
    void SetPanelArt(const std::string& panelName,
        const std::string& artTxtFile) override;
    
    void UpdatePartyDisplay() override;
    void UpdateMonsterDisplay() override;
    void RefreshBattleUI() override;

private:
    // ===== UI 업데이트 함수 =====
    void UpdateSystemLog(class Panel* systemPanel, const std::vector<std::string>& messages);
    
    // 커맨드 패널 업데이트
    void UpdateCommandPanel();
    
    // 인벤토리 패널 업데이트
    void UpdateInventoryPanel(class Panel* inventoryPanel);
 
    // 파티 스탯 패널 업데이트 (전투 중 HP/MP 변화 반영)
    void UpdatePartyPanels();
  
    // 몬스터 정보 패널 업데이트
    void UpdateMonsterInfoPanel();
    
    // 전투 정보 패널 업데이트 (상단 - 스테이지명, 몬스터 HP)
    void UpdateBattleInfoPanel();

    // ===== 헬퍼 함수 =====
    
    // 캐릭터 인덱스로 색상 가져오기 (0=메인, 1~3=동료)
    WORD GetCharacterColor(int partyIndex) const;
    int GetPartyIndex(class Player* player) const;

    // ===== 전투 로직 연동 =====
    
    // BattleManager 턴 처리
    void ProcessBattleTurn();
    
    // 전투 종료 처리
    void EndBattle(bool victory);

    // 로그 받기
    void CollectBattleLogs();

    //  ===== 애니메이션 로드 함수 =====
    ConsoleAnimation LoadAnimation(const std::string& path);
    bool LoadAnimationFromJson(const std::string& fileName);
    void UpdateAnimationPanel();
    void ClearAnimationPanel();

    unsigned __int64 _CurrentAnimFrame = 0;
    float _AnimElapsedTime = 0.0f;
    ConsoleAnimation _CurrentAnimation;
};
