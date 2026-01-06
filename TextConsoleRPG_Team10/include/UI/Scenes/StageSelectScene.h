#pragma once
#include "../UIScene.h"
#include "../../Data/StageData.h"
#include <Windows.h>
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
    std::vector<StageNode> _CurrentNodes;  // 레거시 (사용 안 함)
    int _SelectedNodeIndex;    // 레거시 (사용 안 함)
    int _CurrentStageLevel;    // 레거시 (사용 안 함)

    // ===== 신규 노드 선택 시스템 =====
    std::string _SelectedNodeId;  // 현재 선택 중인 노드 ID
    std::vector<std::string> _AvailableNodeIds;  // 선택 가능한 노드 ID 목록

    // ===== 내부 헬퍼 함수 =====

    // 노드 타입에 따른 아이콘 반환 (레거시)
    std::string GetNodeIcon(ENodeType type) const;

    // 노드 데이터에 따른 아이콘 반환 (EnemyType 고려)
    std::string GetNodeIcon(const NodeData* node) const;

    // 노드 데이터에 따른 색상 반환 (EnemyType 고려)
    WORD GetNodeColor(const NodeData* node) const;
    
    // 선택된 노드인지 확인
    bool IsNodeSelected(const std::string& nodeId) const;
    
    // 선택 가능한 노드 목록 갱신
    void RefreshAvailableNodes();
  
    // 노드 선택 업데이트 및 맵 재렌더링
    void SelectNode(const std::string& nodeId);
  
    // 노드 진입 처리
    void EnterNode(const std::string& nodeId);

public:
    StageSelectScene();
    ~StageSelectScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;

    // ===== UI 업데이트 함수 =====

    void UpdateTowerArrow(Panel* towerPanel, int currentFloor);
    void UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages);
    void UpdateInventoryPanel(Panel* inventoryPanel);
    void UpdateGuidePanel(Panel* guidePanel);  // 가이드 패널 업데이트
    void RenderStageMap(Panel* nodePanel);     // 맵 렌더링

    std::vector<std::string> _SystemLogs; // 시스템 로그 저장 변수 추가
};
