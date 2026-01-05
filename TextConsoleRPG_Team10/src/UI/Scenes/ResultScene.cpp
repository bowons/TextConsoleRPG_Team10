#include "../../../include/UI/Scenes/ResultScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Common/TextColor.h"

ResultScene::ResultScene()
    : UIScene("Result")
    , _ResultType(EResultType::Victory)
    , _InputComplete(false)
{
}

ResultScene::~ResultScene()
{
}

void ResultScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;
    _InputComplete = false;

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 결과 타이틀 패널 (상단) =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 10, 2, 130, 6);
    titlePanel->SetBorder(true, _ResultType == EResultType::Victory ? ETextColor::LIGHT_YELLOW : ETextColor::LIGHT_RED);

    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("");
    titleText->AddLine("");

    if (_ResultType == EResultType::Victory)
    {
        titleText->AddLineWithColor("          [전투 승리/실패 문구] - You Win!",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    }
    else
    {
        titleText->AddLineWithColor("     [전투 승리/실패 문구] - You Lose..",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
    }

    titlePanel->SetContentRenderer(std::move(titleText));
    titlePanel->Redraw();

    // TODO: 결과 타입에 따라 타이틀 동적 변경
  // _ResultType == EResultType::Victory 이면 승리 메시지
    // _ResultType == EResultType::Defeat 이면 패배 메시지

    // ===== 전투 결과 이미지 패널 (중앙 좌측) =====
    Panel* imagePanel = _Drawer->CreatePanel("ResultImage", 10, 10, 80, 30);
    imagePanel->SetBorder(true, ETextColor::CYAN);

    auto imageText = std::make_unique<TextRenderer>();
    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLine("          [전투 결과 이미지]");
    imageText->AddLine("   • 승리 이미지");
    imageText->AddLine("  • 실패 이미지");
    imageText->AddLine("");
    imageText->AddLine("   ------------------------------------");
    imageText->AddLine("");
    imageText->AddLine("            [보상 이미지]");
    imageText->AddLine("   • 골드 이미지");
    imageText->AddLine("           • 경험치 이미지");
    imageText->AddLine("         • 아이템 이미지");
    imageText->AddLine("              • 레벨업 이미지");
    imageText->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    imagePanel->SetContentRenderer(std::move(imageText));
    imagePanel->Redraw();

    // TODO: 여기에서 결과 이미지 조정
    // 승리/패배에 따라 다른 ASCII Art 표시
    //
    // 구현 방법:
    // Panel* imagePanel = _Drawer->GetPanel("ResultImage");
    // auto artRenderer = std::make_unique<AsciiArtRenderer>();
    // std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");
    //
    // std::string fileName = (_ResultType == EResultType::Victory) ? "Victory.txt" : "Defeat.txt";
    // if (artRenderer->LoadFromFile(uiPath, fileName)) {
    //   artRenderer->SetAlignment(ArtAlignment::CENTER);
    //     artRenderer->SetColor(_ResultType == EResultType::Victory ? 
  //         ETextColor::LIGHT_YELLOW : ETextColor::LIGHT_RED);
    //     imagePanel->SetContentRenderer(std::move(artRenderer));
    //     imagePanel->Redraw();
    // }

    // ===== 보상 정보 패널 (중앙 우측) =====
    Panel* rewardPanel = _Drawer->CreatePanel("Reward", 93, 10, 47, 30);
    rewardPanel->SetBorder(true, ETextColor::GREEN);

    auto rewardText = std::make_unique<TextRenderer>();
    rewardText->AddLine("");
    rewardText->AddLineWithColor("  [보상]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    rewardText->AddLine("");
    rewardText->AddLineWithColor("  = 한 정산",
        MakeColorAttribute(ETextColor::CYAN, EBackgroundColor::BLACK));
    rewardText->AddLine("");

    if (_ResultType == EResultType::Victory)
    {
        // 승리 시 보상 표시
        rewardText->AddLine("  • 골드: +0 G");
        rewardText->AddLine("  • 경험치: +0 EXP");
        rewardText->AddLine("  • 아이템 획득:");
        rewardText->AddLine("    - 아이템 이름");
        rewardText->AddLine("");
        rewardText->AddLine("  • 레벨업!");
        rewardText->AddLine("    Lv.1 → Lv.2");
    }
    else
    {
        // 패배 시 메시지
        rewardText->AddLineWithColor("  전투에서 패배했습니다.",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        rewardText->AddLine("");
        rewardText->AddLine("  보상을 받을 수 없습니다.");
    }

    rewardPanel->SetContentRenderer(std::move(rewardText));
    rewardPanel->Redraw();

    // TODO: 보상 정보 동적 업데이트
    // BattleManager 또는 GameManager에서 전투 결과를 가져와 표시합니다.
    //
    // 구현 방법:
    // Panel* rewardPanel = _Drawer->GetPanel("Reward");
    // auto rewardText = std::make_unique<TextRenderer>();
    //
    // if (_ResultType == EResultType::Victory) {
    //     // 몬스터에서 드랍한 보상 가져오기
    //     // auto [gold, exp, item] = monster->DropReward();
    //     //
  //     // rewardText->AddLine("  • 골드: +" + std::to_string(gold) + " G");
    //     // rewardText->AddLine("  • 경험치: +" + std::to_string(exp) + " EXP");
    //     //
    //     // if (item) {
    //     //     rewardText->AddLine("  • 아이템 획득:");
    //     //     rewardText->AddLine("    - " + item->GetName());
    //     // }
    //     //
    //   // // 플레이어에게 보상 적용
    //     // auto player = GameManager::GetInstance()->GetMainPlayer();
    //     // player->GainGold(gold);
    //     // player->GainExp(exp);
    //     //
    //     // // 레벨업 체크
    //     // if (레벨업했으면) {
    //     //     rewardText->AddLine("  • 레벨업!");
    //     //     rewardText->AddLine("    Lv." + std::to_string(oldLevel) + 
    //     //      " → Lv." + std::to_string(newLevel));
    //     // }
    // }
    //
    // rewardPanel->SetContentRenderer(std::move(rewardText));
    // rewardPanel->Redraw();

    // ===== 파티원 정보 패널 (하단 4개) =====
    for (int i = 0; i < 4; ++i)
    {
        int xPos = 10 + (i * 32);
        Panel* memberPanel = _Drawer->CreatePanel("Member" + std::to_string(i + 1), xPos, 41, 30, 4);
        memberPanel->SetBorder(true, ETextColor::CYAN);

        auto memberText = std::make_unique<TextRenderer>();
        memberText->AddLine("");
        memberText->AddLine("  이름 : 유저 | 직업 : 전사");
        memberText->AddLine("  HP 100/200  ATK / DEF");

        memberPanel->SetContentRenderer(std::move(memberText));
        memberPanel->Redraw();
    }

    // TODO: 파티원 정보 동적 업데이트
    // 전투 후 파티원 상태를 표시합니다.
    //
  // 구현 방법:
    // auto party = GameManager::GetInstance()->GetParty();
    // for (size_t i = 0; i < 4; ++i) {
    //     std::string panelID = "Member" + std::to_string(i + 1);
    //     Panel* memberPanel = _Drawer->GetPanel(panelID);
    //
    //     if (i < party.size()) {
    //         auto memberText = std::make_unique<TextRenderer>();
    //         memberText->AddLine("  이름: " + party[i]->GetName() + " | 직업: 전사");
    //         memberText->AddLine("  HP " + std::to_string(party[i]->GetCurrentHP()) + 
    //           "/" + std::to_string(party[i]->GetMaxHP()));
  //         memberPanel->SetContentRenderer(std::move(memberText));
    //     }
    //     memberPanel->Redraw();
    // }

    // ===== 시스템 로그 패널 (하단 중앙) =====
    Panel* logPanel = _Drawer->CreatePanel("SystemLog", 10, 46, 90, 9);
    logPanel->SetBorder(true, ETextColor::LIGHT_CYAN);

    auto logText = std::make_unique<TextRenderer>();
    logText->AddLine("");
    logText->AddLineWithColor("  [ 시스템 로그 출력 창 ]",
        MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));
    logText->AddLine("");

    logPanel->SetContentRenderer(std::move(logText));
    logPanel->Redraw();

    // TODO: 전투 로그 표시
    // 전투 중 발생한 주요 이벤트를 로그로 출력합니다.

  // ===== 인벤토리 & 커맨드 패널 (하단 우측) =====
    Panel* commandPanel = _Drawer->CreatePanel("Command", 103, 46, 37, 9);
    commandPanel->SetBorder(true, ETextColor::WHITE);

    auto commandText = std::make_unique<TextRenderer>();
    commandText->AddLine("");
    commandText->AddLineWithColor("  인벤토리 & 커맨드",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    commandText->AddLine("");

    if (_ResultType == EResultType::Victory)
    {
        commandText->AddLine("  [Enter] 다음 스테이지");
        commandText->AddLine("  [ESC] 메인 메뉴");
    }
    else
    {
        // 패배 시 - 로그라이크이므로 재도전 없음
        commandText->AddLineWithColor("  게임 오버",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        commandText->AddLine("");
        commandText->AddLine("  [Enter] 메인 메뉴");
    }

    commandPanel->SetContentRenderer(std::move(commandText));
    commandPanel->Redraw();

    // TODO: 커맨드 동적 업데이트
    // 승리 시: 다음 스테이지 진행
    // 패배 시: 메인 메뉴로만 이동 (로그라이크 - 재도전 없음)

    _Drawer->Render();
}

void ResultScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void ResultScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void ResultScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void ResultScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;

    int keyCode = input->GetKeyCode();

    if (keyCode == VK_RETURN || keyCode == VK_SPACE)
    {
        if (_ResultType == EResultType::Victory)
        {
            SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
        }
        else
        {
            GameManager::GetInstance()->RestartGame();
        }
    }
    else if (keyCode == VK_ESCAPE)
    {
        if (_ResultType == EResultType::Victory)
        {
            GameManager::GetInstance()->RestartGame();
        }
    }
}
