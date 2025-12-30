#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/BattleManager.h"

using namespace std;

void GameManager::StartGame()
{
    // Implementation needed

    // Main Player 생성
    _MainPlayer = make_shared<Player>();
    // TODO: 플레이어 입력 시스템으로 플레이어 정보 입력 받아 설정
    // 유효한 값으로 입력된 경우 캐릭터 생성 로직 수행

    // Game Loop 시작
    RunMainLoop();
}

void GameManager::RunMainLoop()
{
    // Implementation needed

    // 게임 종료 확인
    while (!_IsGameOver)
    {
        // BattleManager를 통해 전투를 시작
        // TODO: AutoBattle의 결과를 bool 값으로 받아와야 함, 플레이어가 전투에서 패배한 경우 게임 종료
        BattleManager::GetInstance()->StartAutoBattle(_MainPlayer.get());

        // 전투 결과에 따른 보상 계산
        BattleManager::GetInstance()->CalculateReward(_MainPlayer.get());

        // TODO: 플레이어의 레벨 정보를 받아와 게임 종료 확인

        // TODO: 플레이어가 10레벨에 도달하지 않았다면 상점에 방문할 것인지 확인
    }
}

void GameManager::EndGame()
{
    _IsGameOver = true;
}