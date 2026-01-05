#pragma once
#include "../Singleton.h"
#include "../Data/StageData.h"
#include <memory>
#include <vector>
#include <string>

// ===== 스테이지 진행 상태 =====
struct StageProgress
{
    int CurrentFloor = 1;         // 현재 층 (1~10)
    std::string CurrentNodeId = ""; // 현재 위치한 노드 ID

    // 통계 (현재 런에서의 기록)
    int TotalBattlesCompleted = 0;  // 완료한 전투 수
    int NormalMonstersKilled = 0;   // 처치한 일반 몬스터
    int EliteMonstersKilled = 0;    // 처치한 엘리트 몬스터
    int BossesKilled = 0;           // 처치한 보스
    int EventsCleared = 0;      // 클리어한 이벤트 수

    // 방문 기록 (재방문 방지용)
    std::vector<std::string> VisitedNodes;
    
    // 완료된 노드 목록 (전투/이벤트 클리어 후 추가)
    std::vector<std::string> CompletedNodes;
};

// ===== StageManager =====
// 스테이지 진행 상태를 관리하는 매니저
class StageManager : public Singleton<StageManager>
{
private:
    // 현재 진행 상태
    StageProgress _Progress;

    // 현재 층의 노드 데이터 (캐싱)
    std::vector<NodeData> _CurrentFloorNodes;

    // 층 메타 정보 (캐싱)
    std::vector<StageFloorData> _FloorInfoCache;

    // 초기화 여부
    bool _IsInitialized = false;

private:
    StageManager() = default;
    friend class Singleton<StageManager>;

    StageManager(const StageManager&) = delete;
    StageManager& operator=(const StageManager&) = delete;

public:
    // ===== 초기화 및 게임 시작 =====

    // StageManager 초기화
  // FloorInfo 로드 및 캐싱
    // 1층 노드 데이터 로드
    // 시작 노드로 위치 설정
    bool Initialize();

    // 새 게임 시작 (진행 상황 초기화)
    // 1층, 시작 노드로 초기화
    // 통계 초기화
    void StartNewGame();

    // ===== 층 관리 =====

    // 다음 층으로 이동
    // CurrentFloor++
    // 해당 층 노드 로드
    // 시작 노드로 위치 설정
    // return: 성공 시 true, 실패(10층 초과) 시 false
    bool MoveToNextFloor();

    // 현재 층 정보 가져오기
    // return: 현재 층의 StageFloorData (nullptr 가능)
    const StageFloorData* GetCurrentFloorInfo() const;

    // ===== 노드 탐색 및 이동 =====

    // 현재 노드 정보 가져오기
    // return: 현재 위치한 NodeData (nullptr 가능)
    const NodeData* GetCurrentNode() const;

    // 다음 선택 가능한 노드 목록 가져오기
    // 현재 노드의 Connections 참조
    // 각 연결된 노드 ID로 NodeData 조회
    // return: 선택 가능한 NodeData 목록
    std::vector<const NodeData*> GetAvailableNextNodes() const;

    // 특정 노드로 이동
    // nodeId: 이동할 노드 ID
// VisitedNodes에 추가
    // CurrentNodeId 업데이트
    // return: 성공 시 true, 실패(잘못된 ID) 시 false
    bool MoveToNode(const std::string& nodeId);

    // 노드 완료 처리 (전투/이벤트 종료 시 호출)
    // nodeType: 완료한 노드 타입
    // 통계 업데이트 (전투 수, 처치 몬스터 수 등)
    void CompleteNode(ENodeType nodeType);

    // ===== 진행 상태 조회 =====

  // 현재 층 번호
    inline int GetCurrentFloor() const { return _Progress.CurrentFloor; }

    // 현재 노드 ID
    inline const std::string& GetCurrentNodeId() const { return _Progress.CurrentNodeId; }

    // 진행 통계 전체
    inline const StageProgress& GetProgress() const { return _Progress; }

    // 특정 노드 방문 여부
    // nodeId: 확인할 노드 ID
    // return: 방문했으면 true
    bool IsNodeVisited(const std::string& nodeId) const;
    
    // 노드 완료 여부 확인
    // nodeId: 확인할 노드 ID
    // return: 완료했으면 true
    bool IsNodeCompleted(const std::string& nodeId) const;

    // ===== 유틸리티 =====

    // 노드 ID로 NodeData 검색
    // nodeId: 검색할 노드 ID
    // return: 찾은 NodeData (nullptr 가능)
    const NodeData* FindNodeById(const std::string& nodeId) const;

    // 층 번호로 FloorInfo 검색
    // floor: 층 번호 (1~10)
    // return: 찾은 StageFloorData (nullptr 가능)
    const StageFloorData* GetFloorInfo(int floor) const;

    // 현재 층의 모든 노드 가져오기 (UI 렌더링용)
    inline const std::vector<NodeData>& GetCurrentFloorNodes() const { return _CurrentFloorNodes; }

    // ===== 내부 로직 =====

    // Battle 노드에 랜덤하게 Normal/Elite/Boss 타입 배정
    // FloorInfo의 NormalCount, EliteCount, BossCount 기반
    // floor: 배정할 층 번호
    void AssignRandomMonsterTypes(int floor);

    // ===== Getter =====

    inline bool IsInitialized() const { return _IsInitialized; }
};
