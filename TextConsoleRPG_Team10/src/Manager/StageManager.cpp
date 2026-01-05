#include "../../include/Manager/StageManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/GameManager.h"
#include <algorithm>
#include <random>

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

        // 진행 상태 초기화
        _Progress.CurrentFloor = 1; // 여기 변경
        _Progress.CurrentNodeId = "";

        // 1층 노드 로드
        _CurrentFloorNodes = DataManager::GetInstance()->LoadStageNodes(_Progress.CurrentFloor);

        if (_CurrentFloorNodes.empty())
        {
            PrintManager::GetInstance()->PrintLogLine(
                "StageManager::Initialize - Failed to load Floor1.csv",
                ELogImportance::WARNING
            );
            return false;
        }

        // ===== 랜덤 몬스터 타입 배정 =====
        AssignRandomMonsterTypes(_Progress.CurrentFloor);

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
    _CurrentFloorNodes = DataManager::GetInstance()->LoadStageNodes(_Progress.CurrentFloor);

    // ===== 랜덤 몬스터 타입 배정 =====
    AssignRandomMonsterTypes(_Progress.CurrentFloor);

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

    // ===== 랜덤 몬스터 타입 배정 =====
    AssignRandomMonsterTypes(_Progress.CurrentFloor);

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

    // 현재 노드 정보 가져오기
    const NodeData* currentNode = GetCurrentNode();

    // 통계 업데이트 (Battle 노드는 EnemyType으로 구분)
    if (currentNode && currentNode->Type == ENodeType::Battle)
    {
        _Progress.TotalBattlesCompleted++;

        if (currentNode->EnemyType == "Elite")
        {
            _Progress.EliteMonstersKilled++;
        }
        else if (currentNode->EnemyType == "Boss")
        {
            _Progress.BossesKilled++;
        }
        else  // "Normal" 또는 빈 값
        {
            _Progress.NormalMonstersKilled++;
        }
    }
    else
    {
        // Battle이 아닌 다른 타입 (레거시 호환)
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

// ===== 랜덤 몬스터 타입 배정 =====

void StageManager::AssignRandomMonsterTypes(int floor)
{
    // FloorInfo에서 몬스터 수량 정보 가져오기
    const StageFloorData* floorInfo = GetFloorInfo(floor);

    if (!floorInfo)
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::AssignRandomMonsterTypes - FloorInfo not found for floor " + std::to_string(floor),
            ELogImportance::WARNING
        );
        return;
    }

    int normalCount = floorInfo->NormalCount;
    int eliteCount = floorInfo->EliteCount;
    int bossCount = floorInfo->BossCount;

    // Battle 노드만 수집 (EnemyType이 비어있는 노드만)
    std::vector<NodeData*> battleNodes;
    for (auto& node : _CurrentFloorNodes)
    {
        if (node.Type == ENodeType::Battle && node.EnemyType.empty())
        {
            battleNodes.push_back(&node);
        }
    }

    // 배정할 몬스터 타입 풀 생성
    std::vector<std::string> monsterTypePool;

    for (int i = 0; i < normalCount; ++i)
        monsterTypePool.push_back("Normal");

    for (int i = 0; i < eliteCount; ++i)
        monsterTypePool.push_back("Elite");

    for (int i = 0; i < bossCount; ++i)
        monsterTypePool.push_back("Boss");

    // 검증: Battle 노드 수와 몬스터 타입 풀 크기가 일치해야 함
    if (battleNodes.size() != monsterTypePool.size())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "StageManager::AssignRandomMonsterTypes - Mismatch: " +
            std::to_string(battleNodes.size()) + " empty Battle nodes vs " +
            std::to_string(monsterTypePool.size()) + " monster types (Floor " + std::to_string(floor) + ")",
            ELogImportance::WARNING
        );
    }

    // 랜덤 섞기 (Fisher-Yates shuffle)
    for (size_t i = monsterTypePool.size() - 1; i > 0; --i)
    {
        std::uniform_int_distribution<size_t> dist(0, i);
        size_t j = dist(gen);
        std::swap(monsterTypePool[i], monsterTypePool[j]);
    }

    // Battle 노드에 순서대로 배정 (EnemyType만 설정, NodeType은 Battle 유지)
    size_t battleCount = battleNodes.size();
    size_t poolCount = monsterTypePool.size();
    size_t assignCount = (battleCount < poolCount) ? battleCount : poolCount;

    for (size_t i = 0; i < assignCount; ++i)
    {
        // EnemyType만 설정 (NodeType은 Battle로 유지)
        battleNodes[i]->EnemyType = monsterTypePool[i];
    }

    PrintManager::GetInstance()->PrintLogLine(
        "Assigned random monster types for Floor " + std::to_string(floor) +
        " (Normal: " + std::to_string(normalCount) +
        ", Elite: " + std::to_string(eliteCount) +
        ", Boss: " + std::to_string(bossCount) + ")",
        ELogImportance::DISPLAY
    );
}
