#include "../../include/Manager/StageManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include <algorithm>

// ===== 초기화 및 게임 시작 =====

bool StageManager::Initialize()
{
    _IsInitialized = false;

    try
    {
        // FloorInfo 캐싱
        _FloorInfoCache = DataManager::GetInstance()->LoadStageFloorInfo();

        if (_FloorInfoCache.empty())
        {
            PrintManager::GetInstance()->PrintLogLine(
                "StageManager::Initialize - Failed to load FloorInfo.csv",
                ELogImportance::WARNING
            );
            return false;
        }

        // 1층 노드 로드
        _CurrentFloorNodes = DataManager::GetInstance()->LoadStageNodes(1);

        if (_CurrentFloorNodes.empty())
        {
            PrintManager::GetInstance()->PrintLogLine(
                "StageManager::Initialize - Failed to load Floor1.csv",
                ELogImportance::WARNING
            );
            return false;
        }

        // 진행 상태 초기화
        _Progress.CurrentFloor = 1;
        _Progress.CurrentNodeId = "";

        // 시작 노드 찾기
        for (const auto& node : _CurrentFloorNodes)
        {
            if (node.Type == ENodeType::Start)
            {
                _Progress.CurrentNodeId = node.Id;
                break;
            }
        }

        if (_Progress.CurrentNodeId.empty())
        {
            PrintManager::GetInstance()->PrintLogLine(
                "StageManager::Initialize - Start node not found in Floor1",
                ELogImportance::WARNING
            );
            return false;
        }

        _IsInitialized = true;
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager initialized successfully",
            ELogImportance::DISPLAY
        );

        return true;
    }
    catch (const std::exception& ex)
    {
        PrintManager::GetInstance()->PrintLogLine(
            std::string("StageManager::Initialize exception: ") + ex.what(),
            ELogImportance::WARNING
        );
        return false;
    }
}

void StageManager::StartNewGame()
{
    // 진행 상태 초기화
    _Progress = StageProgress();
    _Progress.CurrentFloor = 1;

    // 1층 노드 로드
    _CurrentFloorNodes = DataManager::GetInstance()->LoadStageNodes(1);

    // 시작 노드 찾기
    for (const auto& node : _CurrentFloorNodes)
    {
        if (node.Type == ENodeType::Start)
        {
            _Progress.CurrentNodeId = node.Id;
            break;
        }
    }

    PrintManager::GetInstance()->PrintLogLine(
        "New game started - Floor 1",
        ELogImportance::DISPLAY
    );
}

// ===== 층 관리 =====

bool StageManager::MoveToNextFloor()
{
    if (_Progress.CurrentFloor >= 10)
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::MoveToNextFloor - Already at final floor",
            ELogImportance::WARNING
        );
        return false;
    }

    _Progress.CurrentFloor++;

    // 다음 층 노드 로드
    _CurrentFloorNodes = DataManager::GetInstance()->LoadStageNodes(_Progress.CurrentFloor);

    if (_CurrentFloorNodes.empty())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::MoveToNextFloor - Failed to load Floor" +
            std::to_string(_Progress.CurrentFloor) + ".csv",
            ELogImportance::WARNING
        );
        return false;
    }

    // 시작 노드 찾기
    _Progress.CurrentNodeId = "";
    for (const auto& node : _CurrentFloorNodes)
    {
        if (node.Type == ENodeType::Start)
        {
            _Progress.CurrentNodeId = node.Id;
            break;
        }
    }

    if (_Progress.CurrentNodeId.empty())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::MoveToNextFloor - Start node not found",
            ELogImportance::WARNING
        );
        return false;
    }

    PrintManager::GetInstance()->PrintLogLine(
        "Moved to Floor " + std::to_string(_Progress.CurrentFloor),
        ELogImportance::DISPLAY
    );

    return true;
}

const StageFloorData* StageManager::GetCurrentFloorInfo() const
{
    return GetFloorInfo(_Progress.CurrentFloor);
}

// ===== 노드 탐색 및 이동 =====

const NodeData* StageManager::GetCurrentNode() const
{
    return FindNodeById(_Progress.CurrentNodeId);
}

std::vector<const NodeData*> StageManager::GetAvailableNextNodes() const
{
    std::vector<const NodeData*> result;

    const NodeData* currentNode = GetCurrentNode();
    if (!currentNode)
        return result;

    // 현재 노드가 미완료 전투/이벤트라면 앞으로 못 감
    bool isCurrentNodeBlocking = false;
    
    if (currentNode->Type == ENodeType::Battle || 
        currentNode->Type == ENodeType::Elite || 
        currentNode->Type == ENodeType::Boss ||
        currentNode->Type == ENodeType::Event)
    {
        // 현재 노드가 완료되지 않았다면 앞으로 가는 노드 차단
        if (!IsNodeCompleted(currentNode->Id))
        {
            isCurrentNodeBlocking = true;
        }
    }

    // 현재 노드의 연결된 노드들 찾기
    for (const auto& connId : currentNode->Connections)
    {
        const NodeData* nextNode = FindNodeById(connId);
        if (nextNode)
        {
            // 현재 노드가 미완료 전투/이벤트면 앞으로 가는 노드 제외
            if (isCurrentNodeBlocking)
            {
                // 이미 방문한 노드만 허용 (뒤로 가기)
                if (!IsNodeVisited(nextNode->Id))
                {
                    continue;  // 미방문 노드는 제외 (앞으로 못 감)
                }
            }
        
            result.push_back(nextNode);
        }
    }

    return result;
}

bool StageManager::MoveToNode(const std::string& nodeId)
{
    // 노드 존재 확인
    const NodeData* targetNode = FindNodeById(nodeId);
    if (!targetNode)
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::MoveToNode - Node not found: " + nodeId,
            ELogImportance::WARNING
        );
        return false;
    }

    // 현재 노드에서 이동 가능한지 확인
    const NodeData* currentNode = GetCurrentNode();
    if (currentNode)
    {
        bool isConnected = false;
        for (const auto& connId : currentNode->Connections)
        {
            if (connId == nodeId)
            {
                isConnected = true;
                break;
            }
        }

        if (!isConnected)
        {
            PrintManager::GetInstance()->PrintLogLine(
                "StageManager::MoveToNode - Node is not connected: " + nodeId,
                ELogImportance::WARNING
            );
            return false;
        }
    }

    // 이동 처리
    _Progress.CurrentNodeId = nodeId;

    // 방문 기록 추가 (중복 체크)
    if (!IsNodeVisited(nodeId))
    {
        _Progress.VisitedNodes.push_back(nodeId);
    }

    PrintManager::GetInstance()->PrintLogLine(
        "Moved to node: " + nodeId,
        ELogImportance::DISPLAY
    );

    return true;
}

void StageManager::CompleteNode(ENodeType nodeType)
{
    // 현재 노드를 완료 목록에 추가
    const std::string& currentNodeId = _Progress.CurrentNodeId;

    if (!IsNodeCompleted(currentNodeId))
    {
        _Progress.CompletedNodes.push_back(currentNodeId);
    }

    // 통계 업데이트
    switch (nodeType)
    {
    case ENodeType::Battle:
        _Progress.TotalBattlesCompleted++;
        _Progress.NormalMonstersKilled++;
        break;

    case ENodeType::Elite:
        _Progress.TotalBattlesCompleted++;
        _Progress.EliteMonstersKilled++;
        break;

    case ENodeType::Boss:
        _Progress.TotalBattlesCompleted++;
        _Progress.BossesKilled++;
        break;

    case ENodeType::Event:
        _Progress.EventsCleared++;
        break;

    default:
        break;
    }

    PrintManager::GetInstance()->PrintLogLine(
        "Node completed - Total battles: " + std::to_string(_Progress.TotalBattlesCompleted),
        ELogImportance::DISPLAY
    );
}

// ===== 진행 상태 조회 =====

bool StageManager::IsNodeVisited(const std::string& nodeId) const
{
    return std::find(_Progress.VisitedNodes.begin(),
        _Progress.VisitedNodes.end(),
     nodeId) != _Progress.VisitedNodes.end();
}

bool StageManager::IsNodeCompleted(const std::string& nodeId) const
{
    return std::find(_Progress.CompletedNodes.begin(),
    _Progress.CompletedNodes.end(),
      nodeId) != _Progress.CompletedNodes.end();
}

// ===== 유틸리티 =====

const NodeData* StageManager::FindNodeById(const std::string& nodeId) const
{
    for (const auto& node : _CurrentFloorNodes)
    {
        if (node.Id == nodeId)
        {
            return &node;
        }
    }
    return nullptr;
}

const StageFloorData* StageManager::GetFloorInfo(int floor) const
{
    if (floor < 1 || floor > 10)
        return nullptr;

    // FloorInfo는 1층부터 시작하므로 인덱스는 floor - 1
    size_t index = static_cast<size_t>(floor - 1);

    if (index >= _FloorInfoCache.size())
        return nullptr;

    return &_FloorInfoCache[index];
}
