#pragma once
#include "../UIScene.h"
#include <vector>
#include <string>
#include <Windows.h>  // WORD 타입 정의

// 전투 Scene
class BattleScene : public UIScene
{
private:
    // ===== 전투 상태 =====
    int _CurrentTurn;      // 현재 턴 수
    bool _PlayerTurn;      // true: 플레이어 턴, false: 적 턴
    int _SelectedAction;   // 선택한 행동 (0: 공격, 1: 스킬, 2: 아이템)
    int _SelectedTarget;   // 선택한 대상 인덱스
    bool _BattleEnd;       // 전투 종료 여부

  // ===== 시스템 로그 =====
    std::vector<std::string> _SystemLogs;  // 로그 버퍼

    // ===== 인벤토리 UI =====
    int _SelectedItemSlot;  // 선택된 인벤토리 슬롯 (0~4)
    bool _IsSelectingItem;  // 아이템 선택 모드 활성화 여부
    int _SelectedPartyIndex; // 선택된 파티원 인덱스 (0~3)
    bool _IsCancelMode;    // 예약 취소 모드 여부

  // ===== 애니메이션 =====
    bool _IsPlayingAnimation;  // 애니메이션 재생 중
    float _AnimationTimer;     // 애니메이션 타이머
    std::string _CurrentAnimationName;  // 현재 재생 중인 애니메이션 이름

public:
    BattleScene();
    ~BattleScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;

private:
    // ===== UI 업데이트 함수 (StageSelectScene 패턴) =====
    
    // 시스템 로그 업데이트
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
    
    // 플레이어 포인터로 파티 인덱스 찾기
    int GetPartyIndex(Player* player) const;

    // ===== 애니메이션 =====
    
    // 애니메이션 재생
    void PlayAnimation(const std::string& animationName, float duration = 1.0f);
    
    // 애니메이션 정지
    void StopAnimation();

    // ===== 전투 로직 연동 =====
    
    // BattleManager 턴 처리
    void ProcessBattleTurn();
    
    // 전투 종료 처리
    void EndBattle(bool victory);

    // 로그 받기
    void CollectBattleLogs();
};
