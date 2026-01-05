#pragma once
#include <string>
#include <vector>

// 노드 타입 정의
enum class ENodeType
{
    Start,       // 시작 지점
    Battle,      // 일반 전투 (Normal)
    Elite,       // 엘리트 전투
    Boss,        // 보스 전투
    Event,       // 이벤트 (동료 영입 등)
    Empty,       // 빈 노드 (분기만 제공)
    Exit       // 다음 층으로 (UP)
};

// 개별 노드 데이터
struct NodeData
{
    std::string Id;     // 노드 고유 ID (예: "1-0", "2-3")
    ENodeType Type;         // 노드 타입
    std::string EnemyType;  // 적 타입 ("Normal", "Elite", "Boss")
    int EnemyCount = 0;                // 몬스터 수
    std::string EventType;             // 이벤트 타입 ("Companion", "Shop" 등)
    
    // 시각적 위치 (렌더링용, 옵션)
    int PosX = 0;
    int PosY = 0;
    
    // 연결된 다음 노드 ID 목록 (세미콜론으로 구분: "1-1;1-2")
    std::vector<std::string> Connections;
};

// 층(Floor) 전체 데이터
struct StageFloorData
{
    int Floor;     // 층 번호 (1~10)
    int BaseLevel;          // 기준 레벨
    int BattleCount;       // 전투 수
    int TotalXP;   // 획득 가능한 총 경험치
    std::string Description;  // 설계 의도 / 설명
    
    // ===== 몬스터 수량 관리 (신규 추가) =====
    int NormalCount = 0;    // 일반 몬스터 수
    int EliteCount = 0;     // 엘리트 몬스터 수
    int BossCount = 0;      // 보스 몬스터 수
    
    std::vector<NodeData> Nodes;            // 해당 층의 모든 노드
};

// 전체 스테이지 데이터베이스
struct StageDatabase
{
    std::vector<StageFloorData> Floors;     // 1~10층 데이터
};
