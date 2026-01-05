#include "../../../include/UI/Scenes/CompanionRecruitScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Common/TextColor.h"
#include <Windows.h>
#include "../../../include/Unit/Player.h"

CompanionRecruitScene::CompanionRecruitScene()
    : UIScene("CompanionRecruit")
    , _CompanionName("???")
    , _InputName("")
    , _RecruitAccepted(false)
    , _SelectedOption(0)
{
}

CompanionRecruitScene::~CompanionRecruitScene()
{
}

void CompanionRecruitScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;
    _RecruitAccepted = false;
    _SelectedOption = 0;
    _InputName.clear();

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 타이틀 패널 (상단) =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 0, 0, 122, 5);
    titlePanel->SetBorder(true, ETextColor::LIGHT_CYAN);

    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("");
    titleText->AddLineWithColor("                    [동료 영입 스테이지 관련 문구] - 건곤한 길을 일구",
        MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));
    titleText->AddLine("");

    titlePanel->SetContentRenderer(std::move(titleText));
    titlePanel->Redraw();

    // ===== 동료 캐릭터 이미지 패널 (좌측 중앙) =====
    Panel* characterPanel = _Drawer->CreatePanel("Character", 0, 5, 61, 24);
    characterPanel->SetBorder(true, ETextColor::YELLOW);

    // TODO: DataManager에서 동료 데이터 가져오기
    // const CompanionData* companionData = DataManager::GetInstance()->GetCompanionData(companionId);
    // std::string asciiArtPath = companionData->ascii_file;

    auto characterArt = std::make_unique<AsciiArtRenderer>();
    std::string asciiArtPath =
        DataManager::GetInstance()->GetResourcePath("Characters");
    characterArt->LoadFromFile(asciiArtPath, "P_Warrior.txt");  // 임시

    characterPanel->SetContentRenderer(std::move(characterArt));
    characterPanel->Redraw();

    // ===== 동료 정보 패널 (우측 중앙) =====
    Panel* infoPanel = _Drawer->CreatePanel("CompanionInfo", 61, 5, 61, 24);
    infoPanel->SetBorder(true, ETextColor::GREEN);

    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("");
    infoText->AddLineWithColor("  [동료 이미지]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    infoText->AddLine("");
    infoText->AddLine("  • 각종 능력 실패에 느낌이");
    infoText->AddLine("  • 있어도 랜덤솔 수 있도록");
    infoText->AddLine("");

    // TODO: 동료 정보 동적 표시
    // infoText->AddLine("  이름: " + companionData->name);
    // infoText->AddLine("  직업: " + GetJobName(companionData->job_type));
    // infoText->AddLine("  HP: " + std::to_string(companionData->hp));
    // infoText->AddLine("  공격력: " + std::to_string(companionData->atk));
    // ...

    infoPanel->SetContentRenderer(std::move(infoText));
    infoPanel->Redraw();

    // ===== 타워(맵) 패널 (우측 상단) - StageSelectScene과 동일 =====
    Panel* towerPanel = _Drawer->CreatePanel("Tower", 122, 1, 30, 30);
    towerPanel->SetBorder(true, ETextColor::DARK_GRAY);

    auto towerText = std::make_unique<TextRenderer>();
    towerText->AddLine("");
    towerText->AddLineWithColor("  [맵]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    towerText->AddLine("");
    towerText->AddLine("  = 탑 정식");

    // TODO: 실제 맵 렌더링 로직 추가

    towerPanel->SetContentRenderer(std::move(towerText));
    towerPanel->Redraw();

    // ===== 플레이어 정보 패널 (하단 상단) =====
    Panel* playerPanel = _Drawer->CreatePanel("PlayerInfo", 0, 29, 122, 7);
    playerPanel->SetBorder(true, ETextColor::CYAN);

    auto playerText = std::make_unique<TextRenderer>();
    playerText->AddLine("");

    auto party = GameManager::GetInstance()->GetParty();
    if (!party.empty() && party[0]) {
        Player* player = party[0].get();
        std::string name = player->GetName();
        std::string className = "전사";  // TODO: 직업 정보 추가
        int hp = player->GetCurrentHP();
        int maxHp = player->GetMaxHP();
        int atk = player->GetAtk();
        int def = player->GetDef();

        playerText->AddLine("  이름 : " + name + " | 직업 : " + className);
        playerText->AddLine("  HP " + std::to_string(hp) + "/" + std::to_string(maxHp) + 
                           " ATK " + std::to_string(atk) + " / DEF " + std::to_string(def));
    }

    playerPanel->SetContentRenderer(std::move(playerText));
    playerPanel->Redraw();

    // ===== 시스템 로그 패널 (하단 좌측-중앙), 내부 우측에 커맨드 통합 =====
    Panel* logPanel = _Drawer->CreatePanel("SystemLog", 0, 36, 113, 9);
    logPanel->SetBorder(true, ETextColor::WHITE);

    // 좌측 영역: 시스템 로그 (1 ~ 74)
    auto logText = std::make_unique<TextRenderer>();
    logText->AddLine("");
    logText->AddLineWithColor(
        "  [ 시스템 로그 출력 창 ]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    logText->AddLine("");
    logText->AddLine("  동료를 발견했습니다!");
    logPanel->AddRenderer(1, 0, 74, 10, std::move(logText));

    // 우측 영역: 커맨드 (75 ~ 111)
    auto commandText = std::make_unique<TextRenderer>();
    commandText->AddLine("");
    commandText->AddLineWithColor(
        "  [ 커맨드 ]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    commandText->AddLine("");
    commandText->AddLine("  [←/→] 선택");
    commandText->AddLine("  [Enter] 확인");
    commandText->AddLine("  [ESC] 거부");
    logPanel->AddRenderer(75, 0, 37, 10, std::move(commandText));

    logPanel->Redraw();

    // ===== 인벤토리 & 골드 패널 (하단 우측) =====
    Panel* inventoryCommandPanel = _Drawer->CreatePanel("Command", 113, 36, 37, 9);
    inventoryCommandPanel->SetBorder(true, ETextColor::WHITE);

    auto inventoryCommandText = std::make_unique<TextRenderer>();
    inventoryCommandText->AddLine("");
    inventoryCommandText->AddLineWithColor(
        "  인벤토리 & 골드",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    inventoryCommandText->AddLine("");

    // TODO: 인벤토리 정보 표시
    if (!party.empty() && party[0]) {
        inventoryCommandText->AddLine("  골드: " + std::to_string(party[0]->GetGold()));
    }

    inventoryCommandPanel->SetContentRenderer(std::move(inventoryCommandText));
    inventoryCommandPanel->Redraw();

    _Drawer->Render();
}

void CompanionRecruitScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void CompanionRecruitScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void CompanionRecruitScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void CompanionRecruitScene::HandleInput()
{
    // TODO: 입력 처리 구현
    // VK_LEFT/RIGHT: 영입/거부 선택 전환
    // VK_RETURN: 선택 확인 (영입 시 이름 입력)
    // VK_ESCAPE: 거부
}
