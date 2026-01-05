// UIDrawer 테스트 예제
// 팀원들이 GameScene을 만들 때 참고할 수 있는 종합 예제입니다.
// 애니메이션을 제외한 거의 모든 UIDrawer 기능을 시연합니다.

#include "include/UI/UIDrawer.h"
#include "include/UI/Panel.h"
#include "include/UI/TextRenderer.h"
#include "include/UI/StatRenderer.h"
#include "include/UI/AsciiArtRenderer.h"
#include "include/UI/InputBridge.h"
#include "include/Manager/DataManager.h"
#include "include/Manager/InputManager.h"
#include "include/Manager/PrintManager.h"
#include <Windows.h>
#include <conio.h>
#include <string>
#include <iostream>

// 테스트 씬 1: 기본 패널 및 텍스트 렌더링
void TestScene_BasicPanelsAndText()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 상단 타이틀 패널
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14); // 노란색 테두리
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 1: 기본 패널 및 텍스트 ===");
    titleText->AddLine("");
    titleText->AddLine("여러 패널을 배치하고 텍스트를 렌더링하는 예제입니다.");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 좌측 패널 - 시스템 로그
    Panel* leftPanel = drawer->CreatePanel("LeftLog", 0, 5, 52, 30);
    leftPanel->SetBorder(true, 10); // 초록색 테두리
    auto leftLog = std::make_unique<TextRenderer>();
    leftLog->AddLine("[시스템 로그]");
    leftLog->AddLine("");
    leftLog->AddLine("이것은 좌측 패널입니다.");
    leftLog->AddLine("여러 줄의 텍스트를 표시할 수 있습니다.");
    leftLog->AddLine("");
    leftLog->AddLine("한글도 정상적으로 처리됩니다!");
    leftLog->AddLine("색상도 변경 가능합니다.");
    leftLog->SetTextColor(10);
    leftLog->SetAutoScroll(true);
    leftPanel->SetContentRenderer(std::move(leftLog));

    // 우측 패널 - 정보
    Panel* rightPanel = drawer->CreatePanel("RightInfo", 54, 5, 52, 30);
    rightPanel->SetBorder(true, 11); // 하늘색 테두리
    auto rightText = std::make_unique<TextRenderer>();
    rightText->AddLine("[정보 패널]");
    rightText->AddLine("");
    rightText->AddLine("패널은 절대 좌표로 배치됩니다.");
    rightText->AddLine("X, Y, Width, Height 설정 가능");
    rightText->AddLine("");
    rightText->AddLine("테두리 색상도 자유롭게!");
    rightText->AddLine("SetBorder(true, colorCode)");
    rightText->SetTextColor(11);
    rightPanel->SetContentRenderer(std::move(rightText));

    // 하단 안내 패널
    Panel* bottomPanel = drawer->CreatePanel("Bottom", 0, 35, 106, 5);
    bottomPanel->SetBorder(true, 7); // 밝은 회색 테두리
    auto bottomText = std::make_unique<TextRenderer>();
    bottomText->AddLine("아무 키나 눌러 다음 테스트로 이동...");
    bottomText->SetTextColor(15);
    bottomPanel->SetContentRenderer(std::move(bottomText));

    drawer->Render();

    // InputManager로 키 대기
    while (!inputMgr->IsKeyPressed())
    {
        Sleep(16);
    }
    inputMgr->GetKeyCode(); // 키 소비
}

// 테스트 씬 2: StatRenderer 및 동적 업데이트
void TestScene_StatRendererAndDynamicUpdate()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 2: StatRenderer 및 동적 업데이트 ===");
    titleText->AddLine("");
    titleText->AddLine("스탯을 표시하고 실시간으로 업데이트하는 예제입니다.");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 플레이어 스탯 패널
    Panel* playerPanel = drawer->CreatePanel("PlayerStats", 10, 10, 40, 20);
    playerPanel->SetBorder(true, 10);
    auto playerStats = std::make_unique<StatRenderer>();
    playerStats->SetStat("이름", "테스트 플레이어");
    playerStats->SetStat("레벨", "5");
    playerStats->SetStat("HP", "100/100");
    playerStats->SetStat("MP", "50/50");
    playerStats->SetStat("공격력", "25");
    playerStats->SetStat("방어력", "15");
    playerStats->SetStat("골드", "1000");
    playerStats->SetKeyColor(11);   // 하늘색
    playerStats->SetValueColor(14); // 노란색
    playerPanel->SetContentRenderer(std::move(playerStats));

    // 몬스터 스탯 패널
    Panel* monsterPanel = drawer->CreatePanel("MonsterStats", 56, 10, 40, 20);
    monsterPanel->SetBorder(true, 12);
    auto monsterStats = std::make_unique<StatRenderer>();
    monsterStats->SetStat("이름", "고블린");
    monsterStats->SetStat("레벨", "3");
    monsterStats->SetStat("HP", "80/80");
    monsterStats->SetStat("공격력", "15");
    monsterStats->SetStat("방어력", "8");
    monsterStats->SetKeyColor(12);  // 빨간색
    monsterStats->SetValueColor(15); // 흰색
    monsterPanel->SetContentRenderer(std::move(monsterStats));

    // 로그 패널
    Panel* logPanel = drawer->CreatePanel("BattleLog", 10, 32, 86, 20);
    logPanel->SetBorder(true, 7);
    auto battleLog = std::make_unique<TextRenderer>();
    battleLog->AddLine("[전투 로그]");
    battleLog->AddLine("");
    battleLog->SetTextColor(15);
    logPanel->SetContentRenderer(std::move(battleLog));

    // 안내
    Panel* infoPanel = drawer->CreatePanel("Info", 10, 54, 86, 6);
    infoPanel->SetBorder(true, 7);
    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("스페이스 바를 누르면 전투가 진행됩니다 (5턴)");
    infoText->AddLine("ESC를 누르면 메인 메뉴로 돌아갑니다.");
    infoText->SetTextColor(15);
    infoPanel->SetContentRenderer(std::move(infoText));

    // 최초 렌더링
    drawer->Render();

    // 시뮬레이션 데이터
    int playerHP = 100;
    int monsterHP = 80;
    int turn = 0;

    bool running = true;
    while (running)
    {
        // InputManager의 논블로킹 메서드 사용
        if (inputMgr->IsKeyPressed())
        {
            int key = inputMgr->GetKeyCode();

            if (key == 27) // ESC
            {
                running = false;
            }
            else if (key == 32 && turn < 5) // SPACE
            {
                turn++;

                // 플레이어 공격
                int playerDamage = 15 + (rand() % 11); // 15~25
                monsterHP -= playerDamage;
                if (monsterHP < 0) monsterHP = 0;

                // 몬스터 공격
                int monsterDamage = 0;
                if (monsterHP > 0)
                {
                    monsterDamage = 10 + (rand() % 6); // 10~15
                    playerHP -= monsterDamage;
                    if (playerHP < 0) playerHP = 0;
                }

                // 스탯 업데이트
                StatRenderer* pStats = dynamic_cast<StatRenderer*>(playerPanel->GetContentRenderer());
                if (pStats)
                {
                    pStats->SetStat("HP", std::to_string(playerHP) + "/100");
                }

                StatRenderer* mStats = dynamic_cast<StatRenderer*>(monsterPanel->GetContentRenderer());
                if (mStats)
                {
                    mStats->SetStat("HP", std::to_string(monsterHP) + "/80");
                }

                // 로그 업데이트
                TextRenderer* log = dynamic_cast<TextRenderer*>(logPanel->GetContentRenderer());
                if (log)
                {
                    log->AddLine("=== Turn " + std::to_string(turn) + " ===");
                    log->AddLine("플레이어의 공격! " + std::to_string(playerDamage) + " 데미지!");

                    if (monsterHP > 0)
                    {
                        log->AddLine("고블린의 공격! " + std::to_string(monsterDamage) + " 데미지!");
                    }
                    else
                    {
                        log->AddLine("고블린을 처치했습니다!");
                    }
                    log->AddLine("");
                }

                // 모든 패널 재렌더링 (정적 패널 포함)
                titlePanel->Redraw();
                playerPanel->Redraw();
                monsterPanel->Redraw();
                logPanel->Redraw();
                infoPanel->Redraw();
                drawer->Render();

                // 전투 종료 처리
                if (playerHP <= 0 || monsterHP <= 0)
                {
                    // 안내 패널 업데이트
                    TextRenderer* info = dynamic_cast<TextRenderer*>(infoPanel->GetContentRenderer());
                    if (info)
                    {
                        info->Clear();
                        info->AddLine("전투가 종료되었습니다!");
                        info->AddLine("아무 키나 눌러 메인 메뉴로 돌아갑니다...");
                        infoPanel->Redraw();
                    }

                    // 로그에 종료 메시지 추가
                    TextRenderer* log = dynamic_cast<TextRenderer*>(logPanel->GetContentRenderer());
                    if (log)
                    {
                        log->AddLine("");
                        if (playerHP <= 0)
                        {
                            log->AddLineWithColor("패배했습니다...", 12);
                        }
                        else
                        {
                            log->AddLineWithColor("승리했습니다!", 10);
                        }
                        log->AddLine("");
                        logPanel->Redraw();
                    }

                    // 모든 패널 재렌더링
                    titlePanel->Redraw();
                    playerPanel->Redraw();
                    monsterPanel->Redraw();
                    logPanel->Redraw();
                    infoPanel->Redraw();
                    drawer->Render();

                    // InputManager로 키 대기
                    while (!inputMgr->IsKeyPressed())
                    {
                        Sleep(16);
                    }
                    inputMgr->GetKeyCode(); // 키 소비

                    running = false;
                }
            }
        }
        Sleep(16); // ~60 FPS
    }
}

// 테스트 씬 3: 다양한 색상
void TestScene_Colors()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 3: 다양한 색상 ===");
    titleText->AddLine("");
    titleText->AddLine("패널과 텍스트의 다양한 색상 조합을 테스트합니다.");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 색상 예제 - 파란색
    Panel* bluePanel = drawer->CreatePanel("Blue", 0, 5, 35, 15);
    bluePanel->SetBorder(true, 9); // 밝은 파란색
    auto blueText = std::make_unique<TextRenderer>();
    blueText->AddLine("파란색 패널");
    blueText->AddLine("");
    blueText->AddLine("Color Code: 9");
    blueText->AddLine("LIGHT_BLUE");
    blueText->SetTextColor(9);
    bluePanel->SetContentRenderer(std::move(blueText));

    // 색상 예제 - 초록색
    Panel* greenPanel = drawer->CreatePanel("Green", 35, 5, 36, 15);
    greenPanel->SetBorder(true, 10); // 빨간색
    auto greenText = std::make_unique<TextRenderer>();
    greenText->AddLine("초록색 패널");
    greenText->AddLine("");
    greenText->AddLine("Color Code: 10");
    greenText->AddLine("LIGHT_GREEN");
    greenText->SetTextColor(10);
    greenPanel->SetContentRenderer(std::move(greenText));

    // 색상 예제 - 빨간색
    Panel* redPanel = drawer->CreatePanel("Red", 71, 5, 35, 15);
    redPanel->SetBorder(true, 12); // 밝은 빨간색
    auto redText = std::make_unique<TextRenderer>();
    redText->AddLine("빨간색 패널");
    redText->AddLine("");
    redText->AddLine("Color Code: 12");
    redText->AddLine("LIGHT_RED");
    redText->SetTextColor(12);
    redPanel->SetContentRenderer(std::move(redText));

    // StatRenderer 색상 예제
    Panel* statPanel1 = drawer->CreatePanel("Stat1", 0, 20, 53, 15);
    statPanel1->SetBorder(true, 11);
    auto stat1 = std::make_unique<StatRenderer>();
    stat1->SetStat("키 색상", "하늘색(11)");
    stat1->SetStat("값 색상", "노란색(14)");
    stat1->SetStat("레벨", "10");
    stat1->SetStat("공격력", "50");
    stat1->SetKeyColor(11);
    stat1->SetValueColor(14);
    statPanel1->SetContentRenderer(std::move(stat1));

    Panel* statPanel2 = drawer->CreatePanel("Stat2", 53, 20, 53, 15);
    statPanel2->SetBorder(true, 13);
    auto stat2 = std::make_unique<StatRenderer>();
    stat2->SetStat("키 색상", "마젠타(13)");
    stat2->SetStat("값 색상", "흰색(15)");
    stat2->SetStat("레벨", "10");
    stat2->SetStat("공격력", "50");
    stat2->SetKeyColor(13);
    stat2->SetValueColor(15);
    statPanel2->SetContentRenderer(std::move(stat2));

    // 안내
    Panel* infoPanel = drawer->CreatePanel("Info", 0, 35, 106, 5);
    infoPanel->SetBorder(true, 7);
    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("아무 키나 눌러 다음 테스트로 이동...");
    infoText->SetTextColor(15);
    infoPanel->SetContentRenderer(std::move(infoText));

    drawer->Render();

    // InputManager로 키 대기
    while (!inputMgr->IsKeyPressed())
    {
        Sleep(16);
    }
    inputMgr->GetKeyCode(); // 키 소비
}

// 테스트 씬 4: AsciiArtRenderer (JSON 애니메이션)
void TestScene_AsciiArt()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 4: ASCII Art Animation (JSON) ===");
    titleText->AddLine("");
    titleText->AddLine("JSON 포맷 애니메이션을 표시합니다. (Resources/Animations/test.json)");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // ASCII Art 애니메이션 패널
    Panel* artPanel = drawer->CreatePanel("Art", 8, 8, 90, 45);
    artPanel->SetBorder(true, 12);
    auto art = std::make_unique<AsciiArtRenderer>();

    // Animations 폴더에서 test.json 로드
    std::string animationsPath = DataManager::GetInstance()->GetResourcePath("Animations");
    bool artLoaded = false;

    // test.json 파일 로드 시도
    if (DataManager::GetInstance()->FileExists(animationsPath, "test.json"))
    {
        artLoaded = art->LoadAnimationFromJson(animationsPath, "test.json");
        if (artLoaded)
        {
            art->SetColor(12);
            art->SetFrameDuration(0.3f);  // 프레임당 0.3초
            art->StartAnimation();
        }
    }

    if (!artLoaded)
    {
        // test.json이 없는 경우 기본 텍스트 표시
        auto fallbackText = std::make_unique<TextRenderer>();
        fallbackText->AddLine("[ASCII Art 애니메이션 영역]");
        fallbackText->AddLine("");
        fallbackText->AddLine("Resources/Animations/ 폴더에");
        fallbackText->AddLine("test.json 파일을 추가하면");
        fallbackText->AddLine("애니메이션이 재생됩니다!");
        fallbackText->AddLine("");
        fallbackText->AddLine("JSON 포맷:");
        fallbackText->AddLine("{");
        fallbackText->AddLine("  \"frames\": [");
        fallbackText->AddLine("    [\"frame 1 line 1\", \"frame 1 line 2\"],");
        fallbackText->AddLine("    [\"frame 2 line 1\", \"frame 2 line 2\"]");
        fallbackText->AddLine("  ]");
        fallbackText->AddLine("}");
        fallbackText->SetTextColor(7);
        artPanel->SetContentRenderer(std::move(fallbackText));
    }
    else
    {
        artPanel->SetContentRenderer(std::move(art));
    }

    // 안내
    Panel* infoPanel = drawer->CreatePanel("Info", 0, 55, 106, 5);
    infoPanel->SetBorder(true, 7);
    auto infoText = std::make_unique<TextRenderer>();
    if (artLoaded)
    {
        infoText->AddLine("애니메이션이 재생됩니다. (ESC: 중지, 아무 키: 다음 테스트)");
    }
    else
    {
        infoText->AddLine("test.json 파일을 찾을 수 없습니다. 아무 키나 눌러 다음 테스트로 이동...");
    }
    infoText->SetTextColor(15);
    infoPanel->SetContentRenderer(std::move(infoText));

    drawer->Render();

    // 애니메이션 루프 (artLoaded가 true일 때만)
    if (artLoaded)
    {
        bool running = true;
        while (running)
        {
            drawer->Update();  // 애니메이션 프레임 업데이트

            // ESC 키 체크
            if (inputMgr->IsKeyPressed())
            {
                int key = inputMgr->GetKeyCode();
                if (key == 27)  // ESC
                {
                    running = false;
                }
                else
                {
                    running = false;
                }
            }

            Sleep(16);  // ~60 FPS
        }
    }
    else
    {
        // InputManager로 키 대기
        while (!inputMgr->IsKeyPressed())
        {
            Sleep(16);
        }
        inputMgr->GetKeyCode(); // 키 소비
    }
}

// 테스트 씬 5: 복합 레이아웃 (실전 예제)
void TestScene_ComplexLayout()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 헤더
    Panel* header = drawer->CreatePanel("Header", 0, 0, 106, 3);
    header->SetBorder(true, 11);
    auto headerText = std::make_unique<TextRenderer>();
    headerText->AddLine("[Stage 5-3] 어둠의 숲");
    headerText->SetTextColor(11);
    header->SetContentRenderer(std::move(headerText));

    // 플레이어 정보 (좌측 상단)
    Panel* playerInfo = drawer->CreatePanel("PlayerInfo", 0, 3, 30, 15);
    playerInfo->SetBorder(true, 10);
    auto playerStats = std::make_unique<StatRenderer>();
    playerStats->SetStat("플레이어", "용사 김철수");
    playerStats->SetStat("레벨", "15");
    playerStats->SetStat("HP", "350/400");
    playerStats->SetStat("MP", "120/150");
    playerStats->SetStat("EXP", "1250/2000");
    playerStats->SetKeyColor(10);
    playerStats->SetValueColor(14);
    playerInfo->SetContentRenderer(std::move(playerStats));

    // 퀵 인벤토리 (좌측 하단)
    Panel* inventory = drawer->CreatePanel("Inventory", 0, 18, 30, 12);
    inventory->SetBorder(true, 14);
    auto invText = std::make_unique<TextRenderer>();
    invText->AddLine("[인벤토리]");
    invText->AddLine("");
    invText->AddLine("1. 회복 물약 x3");
    invText->AddLine("2. 마나 물약 x5");
    invText->AddLine("3. 공격력 증가 x1");
    invText->SetTextColor(14);
    inventory->SetContentRenderer(std::move(invText));

    // 중앙 - 전투 영역
    Panel* battleArea = drawer->CreatePanel("BattleArea", 30, 3, 46, 27);
    battleArea->SetBorder(true, 12);
    auto battleText = std::make_unique<TextRenderer>();
    battleText->AddLine("");
    battleText->AddLine("");
    battleText->AddLine("      [전투 영역]");
    battleText->AddLine("");
    battleText->AddLine("    여기에 몬스터 아트나");
    battleText->AddLine("    전투 애니메이션이");
    battleText->AddLine("    표시됩니다.");
    battleText->SetTextColor(12);
    battleArea->SetContentRenderer(std::move(battleText));

    // 몬스터 정보 (우측 상단)
    Panel* monsterInfo = drawer->CreatePanel("MonsterInfo", 76, 3, 30, 15);
    monsterInfo->SetBorder(true, 12);
    auto monsterStats = std::make_unique<StatRenderer>();
    monsterStats->SetStat("적", "다크 고블린");
    monsterStats->SetStat("레벨", "12");
    monsterStats->SetStat("HP", "180/250");
    monsterStats->SetStat("공격력", "45");
    monsterStats->SetStat("방어력", "25");
    monsterStats->SetKeyColor(12);
    monsterStats->SetValueColor(15);
    monsterInfo->SetContentRenderer(std::move(monsterStats));

    // 미니맵/상태 (우측 하단)
    Panel* minimap = drawer->CreatePanel("Minimap", 76, 18, 30, 12);
    minimap->SetBorder(true, 7);
    auto minimapText = std::make_unique<TextRenderer>();
    minimapText->AddLine("[상태]");
    minimapText->AddLine("");
    minimapText->AddLine("턴: 3");
    minimapText->AddLine("시간: 00:45");
    minimapText->AddLine("콤보: x2");
    minimapText->SetTextColor(7);
    minimap->SetContentRenderer(std::move(minimapText));

    // 전투 로그 (하단)
    Panel* battleLog = drawer->CreatePanel("BattleLog", 0, 30, 106, 20);
    battleLog->SetBorder(true, 7);
    auto log = std::make_unique<TextRenderer>();
    log->AddLine("[전투 로그]");
    log->AddLine("");
    log->AddLine("전투가 시작되었습니다!");
    log->AddLine("다크 고블린이 나타났다!");
    log->AddLine("");
    log->AddLine("플레이어의 공격! 다크 고블린에게 70 데미지!");
    log->AddLine("다크 고블린의 반격! 플레이어에게 50 데미지!");
    log->SetTextColor(15);
    log->SetAutoScroll(true);
    battleLog->SetContentRenderer(std::move(log));

    // 명령 입력 (최하단)
    Panel* commandPanel = drawer->CreatePanel("Command", 0, 50, 106, 10);
    commandPanel->SetBorder(true, 11);
    auto commandText = std::make_unique<TextRenderer>();
    commandText->AddLine("[명령]");
    commandText->AddLine("");
    commandText->AddLine("[1] 공격    [2] 스킬    [3] 아이템    [4] 방어");
    commandText->AddLine("");
    commandText->AddLine("이것은 복합 레이아웃 예제입니다. 실제 게임 화면처럼 구성되어 있습니다.");
    commandText->AddLine("");
    commandText->AddLine("아무 키나 눌러 메인 메뉴로 돌아갑니다...");
    commandText->SetTextColor(15);
    commandPanel->SetContentRenderer(std::move(commandText));

    drawer->Render();

    // InputManager로 키 대기
    while (!inputMgr->IsKeyPressed())
    {
        Sleep(16);
    }
    inputMgr->GetKeyCode(); // 키 소비
}

// 테스트 씬 6: InputBridge를 사용한 입력 처리
void TestScene_InputBridge()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 6: InputBridge 입력 처리 ===");
    titleText->AddLine("");
    titleText->AddLine("UIDrawer와 InputManager를 연결하여 입력을 처리하는 예제입니다.");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 정보 패널
    Panel* infoPanel = drawer->CreatePanel("Info", 0, 5, 106, 15);
    infoPanel->SetBorder(true, 11);
    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("[InputBridge 정보]");
    infoText->AddLine("");
    infoText->AddLine("InputBridge는 UIDrawer와 InputManager를 연결합니다.");
    infoText->AddLine("");
    infoText->AddLine("주요 기능:");
    infoText->AddLine("- RequestInput(): 문자열 입력");
    infoText->AddLine("- RequestIntInput(): 정수 입력 (범위 지정 가능)");
    infoText->AddLine("- RequestCharInput(): 문자 입력 (유효한 문자 지정 가능)");
    infoText->AddLine("");
    infoText->AddLine("이 예제는 순차적으로 세 가지 입력을 받습니다.");
    infoText->SetTextColor(11);
    infoPanel->SetContentRenderer(std::move(infoText));

    // 입력 로그 패널
    Panel* logPanel = drawer->CreatePanel("Log", 0, 20, 106, 35);
    logPanel->SetBorder(true, 7);
    auto logText = std::make_unique<TextRenderer>();
    logText->AddLine("[입력 로그]");
    logText->AddLine("");
    logText->SetTextColor(15);
    logPanel->SetContentRenderer(std::move(logText));

    // 안내 패널
    Panel* guidePanel = drawer->CreatePanel("Guide", 0, 55, 106, 5);
    guidePanel->SetBorder(true, 7);
    auto guideText = std::make_unique<TextRenderer>();
    guideText->AddLine("각 입력 단계를 진행합니다. 콘솔 창에서 입력하세요.");
    guideText->SetTextColor(15);
    guidePanel->SetContentRenderer(std::move(guideText));

    drawer->Render();

    // InputBridge 가져오기
    InputBridge* inputBridge = InputBridge::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();

    TextRenderer* log = dynamic_cast<TextRenderer*>(logPanel->GetContentRenderer());

    // 1. 문자열 입력 테스트
    log->AddLine("=== 1단계: 문자열 입력 ===");
    log->AddLine("당신의 이름을 입력하세요:");
    logPanel->Redraw();
    drawer->Render();

    std::string playerName = inputMgr->GetInput("");

    log->AddLine("입력된 이름: " + playerName);
    log->AddLine("");
    logPanel->Redraw();
    drawer->Render();
    Sleep(1000);

    // 2. 정수 입력 테스트
    log->AddLine("=== 2단계: 정수 입력 ===");
    log->AddLine("캐릭터 레벨을 선택하세요 (1-50):");
    logPanel->Redraw();
    drawer->Render();

    int level = inputMgr->GetIntInput("", 1, 50);

    log->AddLine("선택된 레벨: " + std::to_string(level));
    log->AddLine("");
    logPanel->Redraw();
    drawer->Render();
    Sleep(1000);

    // 3. 문자 입력 테스트
    log->AddLine("=== 3단계: 문자 입력 ===");
    log->AddLine("직업을 선택하세요:");
    log->AddLine("[W] 전사, [M] 마법사, [A] 궁수");
    logPanel->Redraw();
    drawer->Render();

    char classChoice = inputMgr->GetCharInput("", "WMAwma");

    std::string className;
    switch (tolower(classChoice))
    {
    case 'w': className = "전사"; break;
    case 'm': className = "마법사"; break;
    case 'a': className = "궁수"; break;
    default: className = "알 수 없음"; break;
    }

    log->AddLine("선택된 직업: " + className);
    log->AddLine("");
    logPanel->Redraw();
    drawer->Render();
    Sleep(1000);

    // 결과 출력
    log->AddLine("=== 입력 완료! ===");
    log->AddLine("캐릭터 정보:");
    log->AddLine("이름: " + playerName);
    log->AddLine("레벨: " + std::to_string(level));
    log->AddLine("직업: " + className);
    log->AddLine("");
    log->AddLine("아무 키나 눌러 메인 메뉴로 돌아갑니다...");
    logPanel->Redraw();
    drawer->Render();

    _getch();
}

// 테스트 씬 7: TextRenderer 고급 기능 (PrintManager 호환) ⭐ NEW!
void TestScene_AdvancedTextRenderer()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 7: TextRenderer 고급 기능 ===");
    titleText->AddLine("");
    titleText->AddLine("PrintManager의 모든 기능을 UIDrawer에서 사용할 수 있습니다!");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 데모 패널 1: Log 중요도
    Panel* logPanel = drawer->CreatePanel("LogDemo", 0, 5, 52, 20);
    logPanel->SetBorder(true, 11);
    auto logText = std::make_unique<TextRenderer>();
    logText->AddLine("[데모 1: Log 중요도]");
    logText->AddLine("");
    logText->AddLogLine("일반 로그 메시지", ELogImportance::NONE);
    logText->AddLogLine("중요한 정보입니다!", ELogImportance::DISPLAY);
    logText->AddLogLine("경고! 위험합니다!", ELogImportance::WARNING);
    logText->AddLine("");
    logText->AddLine("PrintManager의 ELogImportance를");
    logText->AddLine("그대로 사용할 수 있습니다!");
    logPanel->SetContentRenderer(std::move(logText));

    // 데모 패널 2: 색상별 텍스트
    Panel* colorPanel = drawer->CreatePanel("ColorDemo", 54, 5, 52, 20);
    colorPanel->SetBorder(true, 12);
    auto colorText = std::make_unique<TextRenderer>();
    colorText->AddLine("[데모 2: 색상별 텍스트]");
    colorText->AddLine("");
    colorText->AddLineWithColor("빨간색 텍스트", static_cast<WORD>(ETextColor::LIGHT_RED));
    colorText->AddLineWithColor("초록색 텍스트", static_cast<WORD>(ETextColor::LIGHT_GREEN));
    colorText->AddLineWithColor("파란색 텍스트", static_cast<WORD>(ETextColor::LIGHT_BLUE));
    colorText->AddLineWithColor("노란색 텍스트", static_cast<WORD>(ETextColor::LIGHT_YELLOW));
    colorText->AddLine("");
    colorText->AddLine("각 줄마다 다른 색상을");
    colorText->AddLine("지정할 수 있습니다!");
    colorPanel->SetContentRenderer(std::move(colorText));

    // 데모 패널 3: 자동 줄바꿈
    Panel* wrapPanel = drawer->CreatePanel("WrapDemo", 0, 25, 52, 20);
    wrapPanel->SetBorder(true, 10);
    auto wrapText = std::make_unique<TextRenderer>();
    wrapText->AddLine("[데모 3: 자동 줄바꿈]");
    wrapText->AddLine("");
    wrapText->EnableAutoWrap(true);
    wrapText->SetWrapWidth(45);
    wrapText->AddLine("이것은 아주 긴 텍스트입니다. 자동 줄바꿈 기능을 사용하면 지정된 너비를 초과하는 텍스트가 자동으로 다음 줄로 넘어갑니다. PrintManager의 LineLimit 기능과 동일합니다!");
    wrapText->AddLine("");
    wrapText->EnableAutoWrap(false);
    wrapText->AddLine("줄바꿈 비활성화 시:");
    wrapText->AddLine("아주 긴 텍스트도 한 줄에 표시됩니다 (패널 너비 초과 시 잘림)");
    wrapPanel->SetContentRenderer(std::move(wrapText));

    // 데모 패널 4: 타이핑 효과 (설명)
    Panel* typingPanel = drawer->CreatePanel("TypingDemo", 54, 25, 52, 20);
    typingPanel->SetBorder(true, 13);
    auto typingText = std::make_unique<TextRenderer>();
    typingText->AddLine("[데모 4: 타이핑 효과]");
    typingText->AddLine("");
    typingText->AddLine("타이핑 효과는 실시간 업데이트가");
    typingText->AddLine("필요하므로 게임 루프에서 사용됩니다.");
    typingText->AddLine("");
    typingText->AddLine("사용 예시:");
    typingText->AddLineWithColor("renderer->EnableTypingEffect(true);", 11);
    typingText->AddLineWithColor("renderer->SetTypingSpeed(", 11);
    typingText->AddLineWithColor("  ETypingSpeed::Normal);", 11);
    typingText->AddLineWithColor("renderer->AddLineWithTyping(", 11);
    typingText->AddLineWithColor("  \"타이핑될 텍스트\");", 11);
    typingText->AddLine("");
    typingText->AddLine("drawer->Update() 호출 시");
    typingText->AddLine("자동으로 타이핑 애니메이션 재생!");
    typingPanel->SetContentRenderer(std::move(typingText));

    // 안내 패널
    Panel* guidePanel = drawer->CreatePanel("Guide", 0, 45, 106, 15);
    guidePanel->SetBorder(true, 7);
    auto guideText = std::make_unique<TextRenderer>();
    guideText->AddLine("[TextRenderer의 새로운 기능]");
    guideText->AddLine("");
    guideText->AddLogLine("AddLogLine(text, importance)", ELogImportance::DISPLAY);
    guideText->AddLine("  -> PrintManager의 ELogImportance 지원");
    guideText->AddLine("");
    guideText->AddLine("AddLineWithColor(text, color)");
    guideText->AddLine("  -> 줄마다 다른 색상 지정");
    guideText->AddLine("");
    guideText->AddLine("EnableAutoWrap(true) + SetWrapWidth(width)");
    guideText->AddLine("  -> PrintManager의 LineLimit과 동일한 자동 줄바꿈");
    guideText->AddLine("");
    guideText->AddLine("아무 키나 눌러 메인 메뉴로 돌아갑니다...");
    guideText->SetTextColor(15);
    guidePanel->SetContentRenderer(std::move(guideText));

    drawer->Render();

    // InputManager로 키 대기
    while (!inputMgr->IsKeyPressed())
    {
        Sleep(16);
    }
    inputMgr->GetKeyCode(); // 키 소비
}

// 테스트 씬 8: 타이핑 효과 실시간 데모 ⭐ NEW!
void TestScene_TypingEffectDemo()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    // 타이틀
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);
    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("=== UIDrawer 테스트 예제 8: 타이핑 효과 실시간 데모 ===");
    titleText->AddLine("");
    titleText->AddLine("PrintManager의 PrintWithTyping 기능을 UIDrawer에서 사용!");
    titleText->SetTextColor(14);
    titlePanel->SetContentRenderer(std::move(titleText));

    // 타이핑 효과 패널
    Panel* typingPanel = drawer->CreatePanel("Typing", 10, 10, 86, 30);
    typingPanel->SetBorder(true, 11);
    auto typingText = std::make_unique<TextRenderer>();

    // 타이핑 효과 설정
    typingText->EnableTypingEffect(true);
    typingText->SetTypingSpeed(ETypingSpeed::Normal);

    typingText->AddLine("[타이핑 효과 데모]");
    typingText->AddLine("");

    // 타이핑될 텍스트들
    typingText->AddLineWithTyping("안녕하세요! 이것은 타이핑 효과 데모입니다.", 14);
    typingText->AddLineWithTyping("", 15);
    typingText->AddLineWithTyping("PrintManager의 PrintWithTyping 기능을", 11);
    typingText->AddLineWithTyping("TextRenderer에서도 사용할 수 있습니다!", 11);
    typingText->AddLineWithTyping("", 15);
    typingText->AddLineWithTyping("속도는 Slow, Normal, Fast 중 선택 가능합니다.", 10);
    typingText->AddLineWithTyping("", 15);
    typingText->AddLineWithTyping("게임 대화, 스토리 텔링, 전투 로그 등에 활용하세요!", 12);

    typingPanel->SetContentRenderer(std::move(typingText));

    // 안내 패널
    Panel* guidePanel = drawer->CreatePanel("Guide", 10, 40, 86, 10);
    guidePanel->SetBorder(true, 7);
    auto guideText = std::make_unique<TextRenderer>();
    guideText->AddLine("타이핑 효과가 재생됩니다. 완료될 때까지 기다려주세요...");
    guideText->AddLine("");
    guideText->AddLine("(타이핑 완료 후 아무 키나 누르면 메인 메뉴로 돌아갑니다)");
    guideText->SetTextColor(15);
    guidePanel->SetContentRenderer(std::move(guideText));

    // 타이핑 효과 루프
    bool typingComplete = false;
    auto startTime = std::chrono::steady_clock::now();

    while (!typingComplete)
    {
        drawer->Update();  // 타이핑 효과 업데이트 + 렌더링

        // 타이핑 완료 확인 (약 10초 후 자동 완료로 가정)
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        if (elapsed > 15)  // 15초 후 자동 완료
        {
            typingComplete = true;

            // 완료 메시지
            TextRenderer* guide = dynamic_cast<TextRenderer*>(guidePanel->GetContentRenderer());
            if (guide)
            {
                guide->Clear();
                guide->AddLineWithColor("타이핑 효과 완료!", 10);
                guide->AddLine("");
                guide->AddLine("아무 키나 눌러 메인 메뉴로 돌아갑니다...");
                guidePanel->Redraw();
                drawer->Render();
            }
        }

        // InputManager로 ESC 키 체크
        if (inputMgr->IsKeyDown(27))  // ESC
        {
            typingComplete = true;
        }

        Sleep(16);  // ~60 FPS
    }

    // InputManager로 키 대기
    while (!inputMgr->IsKeyPressed())
    {
        Sleep(16);
    }
    inputMgr->GetKeyCode(); // 키 소비
}

// 메인 메뉴
void ShowMainMenu()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    InputManager* inputMgr = InputManager::GetInstance();

    bool running = true;
    while (running)
    {
        drawer->ClearScreen();
        drawer->RemoveAllPanels();

        // 타이틀 패널
        Panel* titlePanel = drawer->CreatePanel("Title", 20, 5, 66, 10);
        titlePanel->SetBorder(true, 14);
        auto titleText = std::make_unique<TextRenderer>();
        titleText->AddLine("");
        titleText->AddLine("========================================");
        titleText->AddLine("    UIDrawer 종합 테스트 예제");
        titleText->AddLine("    Comprehensive Test Example");
        titleText->AddLine("========================================");
        titleText->SetTextColor(14);
        titlePanel->SetContentRenderer(std::move(titleText));

        // 메뉴 패널
        Panel* menuPanel = drawer->CreatePanel("Menu", 20, 15, 66, 35);
        menuPanel->SetBorder(true, 11);
        auto menuText = std::make_unique<TextRenderer>();
        menuText->AddLine("");
        menuText->AddLine("테스트할 예제를 선택하세요:");
        menuText->AddLine("");
        menuText->AddLine("[1] 기본 패널 및 텍스트 렌더링");
        menuText->AddLine("[2] StatRenderer 및 동적 업데이트");
        menuText->AddLine("[3] 다양한 색상");
        menuText->AddLine("[4] ASCII Art 렌더링");
        menuText->AddLine("[5] 복합 레이아웃 (실전 예제)");
        menuText->AddLine("[6] InputBridge 입력 처리");
        menuText->AddLine("[7] TextRenderer 고급 기능 ⭐ NEW!");
        menuText->AddLine("    - Log 중요도, 색상별 텍스트, 자동 줄바꿈");
        menuText->AddLine("[8] 타이핑 효과 실시간 데모 ⭐ NEW!");
        menuText->AddLine("    - PrintManager의 타이핑 효과를 UI에서!");
        menuText->AddLine("");
        menuText->AddLine("[ESC] 종료");
        menuText->SetTextColor(15);
        menuPanel->SetContentRenderer(std::move(menuText));

        drawer->Render();

        // InputManager로 입력 처리
        while (!inputMgr->IsKeyPressed())
        {
            Sleep(16);
        }
        int key = inputMgr->GetKeyCode();

        switch (key)
        {
        case '1':
            TestScene_BasicPanelsAndText();
            break;
        case '2':
            TestScene_StatRendererAndDynamicUpdate();
            break;
        case '3':
            TestScene_Colors();
            break;
        case '4':
            TestScene_AsciiArt();
            break;
        case '5':
            TestScene_ComplexLayout();
            break;
        case '6':
            TestScene_InputBridge();
            break;
        case '7':
            TestScene_AdvancedTextRenderer();
            break;
        case '8':
            TestScene_TypingEffectDemo();
            break;
        case 27: // ESC
            running = false;
            break;
        }
    }
}

// UIDrawer 테스트 메인 함수
void RunUIDrawerTest()
{
    // 1. UIDrawer 초기화 (콘솔 창 크기와 일치: 106x60)
    UIDrawer* drawer = UIDrawer::GetInstance();
    if (!drawer->Initialize(106, 60))  // ✅ 60으로 변경 (콘솔 창과 일치)
    {
        std::cerr << "UIDrawer 초기화 실패!" << std::endl;
        return;
    }

    // 2. UI 모드 활성화
    drawer->Activate();

    // 3. 메인 메뉴 표시
    ShowMainMenu();

    // 4. UI 모드 비활성화 (레거시 모드로 복귀)
    drawer->Deactivate();
    drawer->Shutdown();
}
