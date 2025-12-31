#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/InputManager.h"
#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/BattleManager.h"
#include "../../include/Unit/Player.h"

using namespace std;

void GameManager::StartGame()
{
    // Implementation needed
    PrintManager::GetInstance()->PrintLogLine("==========에레보스 타워: 뒤틀린 성의 종언==========");

    string PlayerName = InputManager::GetInstance()->GetInput("플레이어 이름을 입력하세요: ");
    if (PlayerName.empty())
    {
        PlayerName = "Player";
    }

    // Main Player 생성
    _MainPlayer = make_shared<Player>(PlayerName);
    // TODO: 플레이어 입력 시스템으로 플레이어 정보 입력 받아 설정
    // 유효한 값으로 입력된 경우 캐릭터 생성 로직 수행

    PrintManager::GetInstance()->PrintLogLine("캐릭터 생성 완료: " + _MainPlayer->GetName(), ELogImportance::DISPLAY);

    // Game Loop 시작
    RunMainLoop();
}

void GameManager::RunMainLoop()
{
    // Implementation needed

    // 게임 종료 확인
    while (!_IsGameOver)
    {
        string status = "이름: " + _MainPlayer->GetName() + " | 레벨: " + to_string(_MainPlayer->GetLevel());
        PrintManager::GetInstance()->PrintLogLine(status);

        PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "는 다음 층으로 무거운 발걸음을 옮깁니다..");

        // BattleManager를 통해 전투를 시작
        // TODO: AutoBattle의 결과를 bool 값으로 받아와야 함, 플레이어가 전투에서 패배한 경우 게임 종료
        bool BattleWin = BattleManager::GetInstance()->StartAutoBattle(_MainPlayer.get());

        if (BattleWin == false)
        {
            EndGame();
            // TODO: 게임 패배를 처리합니다.
            PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "의 여정은 끝나고 말았습니다...");

            break;
        }

        // TODO: 플레이어의 레벨 정보를 받아와 게임 종료 확인
        if (_MainPlayer->GetLevel() >= 10)
        {
            EndGame();
            // TODO: 게임 승리를 처리합니다.
            PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "는 힘든 여정을 거쳐 무사히 탑의 정상에 도착했습니다..");
            PrintManager::GetInstance()->PrintLogLine("이 거대한 탑의 정상은 너무 고요합니다.. 수상할 정도로 말이죠...");
            break;
        }

        // TODO: 플레이어가 10레벨에 도달하지 않았다면 상점에 방문할 것인지 확인
        // 상점 기능은 도전기능에 해당하므로 여기서는 우선 HealPotion과 AttackUp의 갯수만 1개씩 증가 시키는 것으로 대체합니다.
        // 상점을 방문하지 않으면 즉시 전투를 다시 시작함

        PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "는 무사히 전투를 마쳤습니다.");
        char choice = InputManager::GetInstance()->GetCharInput("다음 층으로 향하기 전 상점에 방문하시겠습니까? [Y] Yes, [N] No", "yYnY");
        char lowerChoice = static_cast<char>(tolower(choice));

        if (lowerChoice == 'y')
        {
            ShopManager* Shop = ShopManager::GetInstance();
            Shop->PrintShop();
            Shop->BuyItem(_MainPlayer.get(), 0); // 상점에서 체력 회복 포션 1개 구매
            Shop->BuyItem(_MainPlayer.get(), 1); // 상점에서 공격력 증가 포션 1개 구매
            PrintManager::GetInstance()->PrintLogLine("든든한 채비가 보험인 법이죠, 두둑한 주머니와 함께 " + _MainPlayer->GetName() + "는 다음 층으로 향합니다.");
        }
        else if (lowerChoice == 'n')
            PrintManager::GetInstance()->PrintLogLine("역시.. 돈은 아끼고 봐야죠, " + _MainPlayer->GetName() + "는 다음 층으로 향합니다.");

    }
}

void GameManager::EndGame()
{
    _IsGameOver = true;
}