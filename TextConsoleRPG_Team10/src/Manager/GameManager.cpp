#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/InputManager.h"
#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/BattleManager.h"
#include "../../include/Unit/Player.h"

void GameManager::StartGame()
{
    // Implementation needed
    PrintManager::GetInstance()->PrintLogLine("==========에레보스 타워: 뒤틀린 성의 종언==========");

    std::string PlayerName = InputManager::GetInstance()->GetInput("플레이어 이름을 입력하세요: ");
    if (PlayerName.empty())
    {
        PlayerName = "Player";
    }

    // Main Player 생성
    _MainPlayer = std::make_shared<Player>(PlayerName);
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
        std::string status = "이름: " + _MainPlayer->GetName() + " | 레벨: " + std::to_string(_MainPlayer->GetLevel());
        PrintManager::GetInstance()->PrintLogLine(status);

        PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "은(는) 다음 층으로 무거운 발걸음을 옮깁니다..");
        PrintManager::GetInstance()->EndLine();

        // BattleManager를 통해 전투를 시작
        // TODO: AutoBattle의 결과를 bool 값으로 받아와야 함, 플레이어가 전투에서 패배한 경우 게임 종료
        bool BattleWin = BattleManager::GetInstance()->StartAutoBattle(_MainPlayer.get());

        if (BattleWin == false)
        {
            EndGame();
            // TODO: 게임 패배를 처리합니다.
            PrintManager::GetInstance()->ChangeTextColor(ETextColor::RED);
            PrintManager::GetInstance()->SetTypingSpeed(ETypingSpeed::Slow);
            PrintManager::GetInstance()->PrintWithTyping(_MainPlayer->GetName() + "의 여정은 끝나고 말았습니다...");
            break;
        }

        // TODO: 플레이어의 레벨 정보를 받아와 게임 종료 확인
        if (_MainPlayer->GetLevel() >= 10)
        {
            bool bossBattleWin = BattleManager::GetInstance()->StartBossBattle(_MainPlayer.get());
            if (bossBattleWin == false)
            {
                EndGame();
                // TODO: 게임 패배를 처리합니다.
            }
            PrintManager::GetInstance()->PrintLogLine("에레보스 타워의 최종 보스 '에테르노'를 물리쳤습니다!!");

            EndGame();
            // TODO: 게임 승리를 처리합니다.
            PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "은(는) 힘든 여정을 거쳐 무사히 탑의 정상에 도착했습니다..");
            PrintManager::GetInstance()->PrintLogLine("이 거대한 탑의 정상은 너무 고요합니다.. 수상할 정도로 말이죠...");
            break;
        }

        // TODO: 플레이어가 10레벨에 도달하지 않았다면 상점에 방문할 것인지 확인
        // 상점 기능은 도전기능에 해당하므로 여기서는 우선 HealPotion과 AttackUp의 갯수만 1개씩 증가 시키는 것으로 대체합니다.
        // 상점을 방문하지 않으면 즉시 전투를 다시 시작함

        PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "은(는) 무사히 전투를 마쳤습니다.");
        PrintManager::GetInstance()->EndLine();
        PrintManager::GetInstance()->PrintWithTyping("다음 층으로 향하기 전 상점에 방문하시겠습니까? ");
        char choice = InputManager::GetInstance()->GetCharInput("[Y] Yes, [N] No ", "yYnN");
        char lowerChoice = static_cast<char>(tolower(choice));

        if (lowerChoice == 'y')
        {
            PrintManager::GetInstance()->EndLine();
            ShopManager* Shop = ShopManager::GetInstance();
            Shop->ReopenShop();

            // 상점 메뉴 루프
            bool stayInShop = true;
            while (stayInShop)
            {
                Shop->PrintShop();
                PrintManager::GetInstance()->EndLine();
                PrintManager::GetInstance()->PrintLog("현재 소지금: ");
                PrintManager::GetInstance()->ChangeTextColor(
                    ETextColor::YELLOW);
                PrintManager::GetInstance()->PrintLog(
                    std::to_string(_MainPlayer->GetGold()));
                PrintManager::GetInstance()->PrintLog(" G");
                PrintManager::GetInstance()->EndLine();

                PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                PrintManager::GetInstance()->PrintLogLine("[1] 아이템 구매");
                PrintManager::GetInstance()->PrintLogLine("[2] 아이템 판매");
                PrintManager::GetInstance()->PrintLogLine("[3] 상점 나가기");
                PrintManager::GetInstance()->EndLine();

                int shopChoice =
                    InputManager::GetInstance()->GetIntInput("선택: ", 1, 3);
                PrintManager::GetInstance()->EndLine();

                if (shopChoice == 1) {
                    // 아이템 구매
                    int itemChoice = InputManager::GetInstance()->GetIntInput(
                        "구매할 아이템 번호를 선택하세요 (취소: 0): ", 0, static_cast<int>(Shop->GetSellListSize()));

                    if (itemChoice > 0)
                    {
                        Shop->BuyItem(_MainPlayer.get(), itemChoice - 1);
                    }
                    else
                    {
                        PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_BLUE);
                        PrintManager::GetInstance()->PrintLogLine("구매를 취소했습니다.");
                        PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                    }
                    PrintManager::GetInstance()->EndLine();
                }
                else if (shopChoice == 2) {
                    // 아이템 판매
                    PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_CYAN);
                    PrintManager::GetInstance()->PrintLogLine("===== 내 인벤토리 =====");
                    Inventory& inventory = _MainPlayer->GetInventory();
                    bool hasItems = false;
                    for (int i = 0; i < 10; ++i) {
                        IItem* item = inventory.GetItemAtSlot(i);
                        int amount = inventory.GetSlotAmount(i);

                        if (item && amount > 0)
                        {
                            hasItems = true;
                            int sellPrice = static_cast<int>(item->GetPrice() * 0.6);
                            std::string msg = std::to_string(i + 1) + ". " + item->GetName() + " x" +
                                std::to_string(amount) + " ( 판매가: " + std::to_string(sellPrice) + " G )";
                            PrintManager::GetInstance()->PrintLogLine(msg);
                        }
                    }

                    if (!hasItems)
                    {
                        PrintManager::GetInstance()->ChangeTextColor(ETextColor::RED);
                        PrintManager::GetInstance()->PrintLogLine("판매할 아이템이 없습니다.");
                        PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                        PrintManager::GetInstance()->EndLine();
                    }
                    else
                    {
                        PrintManager::GetInstance()->EndLine();
                        PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                        int slotChoice = InputManager::GetInstance()->GetIntInput(
                            "판매할 아이템 슬롯 번호를 선택하세요 (취소: 0): ", 0, 10);
                        if (slotChoice > 0)
                        {
                            Shop->SellItem(_MainPlayer.get(), slotChoice - 1);
                        }
                        else
                        {
                            PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_BLUE);
                            PrintManager::GetInstance()->PrintLogLine("판매를 취소했습니다.");
                            PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                        }
                        PrintManager::GetInstance()->EndLine();
                    }
                }
                else if (shopChoice == 3)
                {
                    // 상점 나가기
                    stayInShop = false;
                    PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_BLUE);
                    PrintManager::GetInstance()->PrintLogLine(
                        "상점을 나갑니다.");
                    PrintManager::GetInstance()->ChangeTextColor(ETextColor::LIGHT_GRAY);
                }
            }
            PrintManager::GetInstance()->PrintLogLine("든든한 채비가 보험인 법이죠.");
            PrintManager::GetInstance()->PrintLogLine("두둑한 주머니와 함께 " + _MainPlayer->GetName() + "은(는) 다음 층으로 향합니다.");
        }
        else if (lowerChoice == 'n')
        {
            PrintManager::GetInstance()->PrintLogLine("역시.. 돈은 아끼고 봐야죠.");
            PrintManager::GetInstance()->PrintLogLine(_MainPlayer->GetName() + "은(는) 다음 층으로 향합니다.");
        }
        PrintManager::GetInstance()->EndLine();
    }
}

void GameManager::EndGame()
{
    _IsGameOver = true;
}