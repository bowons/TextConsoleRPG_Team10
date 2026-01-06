#pragma once
#include "../UIScene.h"
#include "../../Data/CompanionData.h"
#include <string>
#include <optional>

class Panel;

// 동료 영입 Scene
class CompanionRecruitScene : public UIScene
{
private:
    std::optional<CompanionData> _CurrentCompanion;  // 현재 등장한 동료 데이터
    int _CompanionLevel;                             // 동료 레벨 (현재 층수)
    std::string _InputName;                          // 사용자가 입력한 이름
    bool _RecruitAccepted;                           // 영입 수락 여부
    int _SelectedOption;                             // 0: 영입, 1: 거부

    // 타워 화살표 업데이트 (StageSelectScene과 동일)
    void UpdateTowerArrow(Panel* towerPanel, int currentFloor);
    
    // 선택 UI 업데이트
    void UpdateSelectionUI();
    
    // 동료 정보 패널 업데이트
    void UpdateCompanionInfoPanel(Panel* infoPanel);

public:
    CompanionRecruitScene();
    ~CompanionRecruitScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
};
