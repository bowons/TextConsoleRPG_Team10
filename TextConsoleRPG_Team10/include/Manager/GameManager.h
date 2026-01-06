#pragma once
#include "../Singleton.h"
#include <memory>
#include <vector>
#include <random>
#include <string>

static std::mt19937 gen(std::random_device{}());

class Player;

class GameManager : public Singleton<GameManager>
{
private:
    std::shared_ptr<Player> _MainPlayer;  // 레거시 호환용
    std::vector<std::shared_ptr<Player>> _Party;  // 파티 멤버 (0번: 메인 플레이어)
    bool _IsGameOver = false;
    bool _IsRunning = false;
    bool _IsInitialized = false;  // 중복 초기화 방지

    // 플레이어 이름 임시 저장 (CharacterSelectScene 연동용)
    std::string _TempPlayerName;

private:
    GameManager() = default;
    friend class Singleton<GameManager>;

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

public:
    // ===== Scene 기반 게임 시스템 =====
    void Initialize();   // 씬 등록
    void StartGame();    // 메인 루프 시작
    void EndGame();  // 게임 종료
    void RestartGame();  // 게임 재시작 (모든 데이터 초기화)

    // 게임 상태 전체 리셋 (패배 시 호출)
    // 모든 Manager, 리소스, 파티, 상태 초기화
    void ResetGameState();


    // ===== 파티 관리 =====
    // 메인 플레이어 설정 (인벤토리 활성화)
    inline void SetMainPlayer(std::shared_ptr<Player> player)
    {
        _Party.clear();
        _Party.push_back(player);
        _MainPlayer = player;  // 레거시 호환
    }

    // 동료 추가 (인벤토리 비활성화)
    inline void AddCompanion(std::shared_ptr<Player> companion)
    {
        if (!_Party.empty() && companion)
        {
            _Party.push_back(companion);
        }
    }

    // 메인 플레이어 가져오기 (0번 인덱스)
    inline std::shared_ptr<Player> GetMainPlayer() const { return _Party.empty() ? nullptr : _Party[0]; }

    // 전체 파티 가져오기
    inline const std::vector<std::shared_ptr<Player>>& GetParty() const { return _Party; }

    // 파티 초기화
    inline void ClearParty()
    {
        _Party.clear();
        _MainPlayer = nullptr;
    }

    // 사망한 동료 제거
    void RemoveDeadCompanions();

    // 특정 동료 제거 (인덱스)
    inline bool RemoveCompanion(size_t index)
    {
        if (index >= _Party.size() || index == 0)  // 메인 플레이어는 제거 불가
            return false;

        _Party.erase(_Party.begin() + index);
        return true;
    }

    // 파티원 수 반환
    inline size_t GetPartySize() const { return _Party.size(); }

    // 생존한 파티원 수 반환 
    size_t GetAliveCount() const;

    // ===== Getter/Setter =====
    inline bool IsRunning() const { return _IsRunning; }
    inline bool IsGameOver() const { return _IsGameOver; }

    // 메인 플레이어 생성 (기존 방식 - 임시로 Warrior)
    void CreateMainPlayer(const std::string& name);
    
    // CSV 기반 메인 플레이어 생성
    // name: 플레이어 이름
    // classId: 직업 ID ("warrior", "mage", "archer", "priest")
    // return: 성공 시 true, 실패 시 false
    bool CreateMainPlayerWithClass(const std::string& name, const std::string& classId);
    
    // 배틀 테스트
    void StartBattleTest();

    // 플레이어 이름 임시 저장/조회
    inline void SetTempPlayerName(const std::string& name) { _TempPlayerName = name; }
    inline std::string GetTempPlayerName() const { return _TempPlayerName; }
    
    // 동료 영입 테스트
    void StartCompanionRecruitTest();
};
