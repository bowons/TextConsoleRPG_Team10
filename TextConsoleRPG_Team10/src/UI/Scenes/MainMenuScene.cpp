#include "../../../include/UI/Scenes/MainMenuScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/PrintManager.h"
#include "../../../include/Manager/SoundPlayer.h"
#include "../../../include/Common/TextColor.h"
#include <Windows.h>

MainMenuScene::MainMenuScene()
    : UIScene("MainMenu")
{
}

MainMenuScene::~MainMenuScene()
{
}

void MainMenuScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;

    // 타이틀 패널 (150x45)
    Panel* titlePanel = _Drawer->CreatePanel("Title", 0, 0, 150, 45);
    titlePanel->SetBorder(true, ETextColor::WHITE);

    auto titleArt = std::make_unique<AsciiArtRenderer>();
    std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");

    // Title.txt 로드 시도
    bool titleLoaded = titleArt->LoadFromFile(uiPath, "Title.txt");

    if (titleLoaded)
    {
        // 성공: AsciiArtRenderer 사용
        titleArt->SetAlignment(ArtAlignment::LEFT);
        titleArt->SetColor(ETextColor::WHITE);
        titlePanel->SetContentRenderer(std::move(titleArt));
    }
    else
    {
        // 실패: 간단한 기본 타이틀 표시
        auto errorText = std::make_unique<TextRenderer>();
        errorText->AddLine("");
        errorText->AddLine("");
        errorText->AddLineWithColor("[ Title.txt not found ]", static_cast<WORD>(ETextColor::LIGHT_RED));
        errorText->AddLine("");
        errorText->AddLine("=== EREBOS TOWER ===");
        errorText->AddLine("The End of Twisted Castle");
        errorText->SetTextColor(static_cast<WORD>(ETextColor::LIGHT_YELLOW));
        titlePanel->SetContentRenderer(std::move(errorText));
    }
    SoundPlayer::GetInstance()->PlaySFX("Player_Hit_Critical");

    // "Press Any Key" 메시지 추가 (플리커링 효과)
    auto pressKeyText = std::make_unique<TextRenderer>();
    pressKeyText->AddLine("");

    // 배경색이 있는 텍스트 (흰 글자 + 파란 배경)
    WORD normalColor = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);
    WORD flickerColor = MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::RED);

    pressKeyText->AddLineWithColor(">>> Press Any Key to Start Game <<<", normalColor);

    // 플리커링 활성화 (0.5초 간격, 노란글자+빨간배경으로 깜빡임)
    pressKeyText->EnableFlicker(true, 0.5f, flickerColor);

    titlePanel->AddRenderer(50, 38, 80, 5, std::move(pressKeyText));

    _Drawer->Render();
}

void MainMenuScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void MainMenuScene::Update()
{
    if (_IsActive)
    {
        // UIDrawer 업데이트 (플리커링 등 애니메이션 처리)
        _Drawer->Update();
        
        // 입력 처리
        HandleInput();
    }
}

void MainMenuScene::Render()
{
    // UIDrawer::Update()에서 자동으로 Render() 호출하므로
    // 여기서는 필요 시에만 강제 렌더링
    if (!_IsActive) {
      _Drawer->Render();
    }
}

void MainMenuScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();

    // 아무 키나 눌리면 다음 씬으로 이동
    if (input->IsKeyPressed())
    {
        input->GetKeyCode();  // 키 소비

        // PlayerNameInput 씬으로 이동
        SceneManager::GetInstance()->ChangeScene(ESceneType::PlayerNameInput);
    }
}
