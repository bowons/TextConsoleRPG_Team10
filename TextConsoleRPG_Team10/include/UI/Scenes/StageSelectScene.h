#pragma once
#include "../UIScene.h"
#include "../../Data/StageData.h"
#include <vector>
#include <string>

class Panel;
// 스테이지 노드 정보 (UI 표시용)
struct StageNode
{
    ENodeType Type;
    std::string Name;
    int Index;
};

// 스테이지 선택 Scene
class StageSelectScene : public UIScene
{
private:
    std::vector<StageNode> _CurrentNodes;  // 현재 선택 가능한 노드들
    int _SelectedNodeIndex;    // 선택한 노드 인덱스
    int _CurrentStageLevel;          // 현재 스테이지 레벨

public:
    StageSelectScene();
    ~StageSelectScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
    void UpdateTowerArrow(Panel* towerPanel, int currentFloor);  // 추가
    void UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages);    // 시스템 로그 업데이트 (다른 씬에서도 사용 가능)
    void UpdateInventoryPanel(Panel* inventoryPanel);    // 인벤토리 패널 업데이트 (다른 씬에서도 사용 가능)
};
