#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/BattleManager.h"
#include "../../include/Unit/Player.h"

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
        bool BattleWin = BattleManager::GetInstance()->StartAutoBattle(_MainPlayer.get());

        if (BattleWin == false)
        {
            EndGame();
            // TODO: 게임 패배를 처리합니다.

            break;
        }

        // 전투 결과에 따른 보상 계산 및 지급, 플레이어는 경험치 및 골드를 획득합니다.
        BattleManager::GetInstance()->CalculateReward(_MainPlayer.get());

        // TODO: 플레이어의 레벨 정보를 받아와 게임 종료 확인
        if (_MainPlayer->GetLevel() >= 10)
        {
            EndGame();
            // TODO: 게임 승리를 처리합니다.
            
            break;
        }

        // TODO: 플레이어가 10레벨에 도달하지 않았다면 상점에 방문할 것인지 확인
        // 상점 기능은 도전기능에 해당하므로 여기서는 우선 HealPotion과 AttackUp의 갯수만 1개씩 증가 시키는 것으로 대체합니다.
        // 상점을 방문하지 않으면 즉시 전투를 다시 시작함

    }
}

void GameManager::EndGame()
{
    _IsGameOver = true;
}