#include "../../../include/UI/Scenes/PlayerNameInputScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/PrintManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"
#include <Windows.h>

PlayerNameInputScene::PlayerNameInputScene()
    : UIScene("PlayerNameInput")
{
}

PlayerNameInputScene::~PlayerNameInputScene()
{
}

void PlayerNameInputScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;
    _PlayerName.clear();

    Panel* infoPanel = _Drawer->CreatePanel("Info", 35, 15, 80, 8);
    infoPanel->SetBorder(true, ETextColor::WHITE);

    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("");
    infoText->AddLine("       [유저 이름 입력 관련 안내 문구]");
    infoText->AddLine("");
    infoText->AddLine("       당신의 이름을 입력해주세요 (최대 10자)");
    infoText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    infoPanel->SetContentRenderer(std::move(infoText));

    Panel* inputPanel = _Drawer->CreatePanel("Input", 35, 25, 80, 8);
    inputPanel->SetBorder(true, ETextColor::WHITE);

    auto inputText = std::make_unique<TextRenderer>();
    inputText->AddLine("");
    inputText->AddLine("    [플레이어 이름 입력 관련 영역]");
    inputText->AddLine("");
    inputText->AddLine("      > ");
    inputText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    inputPanel->SetContentRenderer(std::move(inputText));

    _Drawer->Render();

    Sleep(200);

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hInput);

    Sleep(100);

    while (true)
    {
        _PlayerName = _Input->GetInputAt(43, 29, 10, true);

        if (!_PlayerName.empty())
        {
            break;
        }

        Sleep(50);
    }

    ShowConfirmation();
}

void PlayerNameInputScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void PlayerNameInputScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
    }
}

void PlayerNameInputScene::Render()
{
}

void PlayerNameInputScene::HandleInput()
{
}

void PlayerNameInputScene::ShowConfirmation()
{
    _Drawer->RemoveAllPanels();

    Panel* confirmPanel = _Drawer->CreatePanel("Confirm", 35, 18, 80, 10);
    confirmPanel->SetBorder(true, ETextColor::WHITE);

    auto confirmText = std::make_unique<TextRenderer>();
    confirmText->AddLine("");
    confirmText->AddLine("     모험가 " + _PlayerName + "의 여정이 시작됩니다...");
    confirmText->AddLine("");
    confirmText->AddLine("     [엔터 키를 눌러 계속...]");
    confirmText->SetTextColor(static_cast<WORD>(ETextColor::LIGHT_YELLOW));
    confirmPanel->SetContentRenderer(std::move(confirmText));

    _Drawer->Render();

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hInput);

    wchar_t wbuffer[2] = { 0 };
    DWORD charactersRead = 0;
    ReadConsoleW(hInput, wbuffer, 1, &charactersRead, NULL);

    FlushConsoleInputBuffer(hInput);

    GameManager::GetInstance()->CreateMainPlayer(_PlayerName);

    _IsActive = false;
    Exit();

    // BattleScene 테스트를 위해 바로 전환
    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
}
