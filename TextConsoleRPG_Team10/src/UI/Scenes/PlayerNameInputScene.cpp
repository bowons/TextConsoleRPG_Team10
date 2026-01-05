#include "../../../include/UI/Scenes/PlayerNameInputScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/PrintManager.h"
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

    // 중앙 안내 패널
    Panel* infoPanel = _Drawer->CreatePanel("Info", 35, 15, 80, 8);
    infoPanel->SetBorder(true, ETextColor::WHITE);

    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("");
    infoText->AddLine("       [유저 이름 입력 관련 안내 문구]");
    infoText->AddLine("");
    infoText->AddLine("       당신의 이름을 입력해주세요 (최대 10자)");
    infoText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    infoPanel->SetContentRenderer(std::move(infoText));

    // 플레이어 이름 입력 영역
    Panel* inputPanel = _Drawer->CreatePanel("Input", 35, 25, 80, 8);
    inputPanel->SetBorder(true, ETextColor::WHITE);

    auto inputText = std::make_unique<TextRenderer>();
    inputText->AddLine("");
    inputText->AddLine("      [플레이어 이름 입력 관련 영역]");
    inputText->AddLine("");
    inputText->AddLine("      > ");
    inputText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    inputPanel->SetContentRenderer(std::move(inputText));

    _Drawer->Render();
    
    // Enter에서 바로 입력 처리 (동기 입력)
    HandleInput();
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
    // UIDrawer::Update()에서 자동 렌더링
}

void PlayerNameInputScene::HandleInput()
{
    // InputManager를 통해 좌표 기반 입력 (동기/블로킹)
    // Panel 위치: (35, 25)
    // SetContentRenderer는 이제 (0, 0)부터 시작 (여백 없음)
    // 테두리는 렌더러가 덮어쓰므로 무시
    // 첫 줄 (빈 줄): 25
    // 둘째 줄 ("[...]"): 26
    // 셋째 줄 (빈 줄): 27
    // 넷째 줄 ("> "): 28
    // "      > " = 공백 6칸 + "> " 2칸 → X: 35 + 6 + 2 = 43

    // maxLength = 10: 최대 10칸 (영문 10자 또는 한글 5자)
    _PlayerName = _Input->GetInputAt(43, 29, 10, true);

    // 빈 이름 처리
    if (_PlayerName.empty())
    {
        _PlayerName = "Player";
    }

    // 확인 화면 표시
    ShowConfirmation();
}

void PlayerNameInputScene::ShowConfirmation()
{
    // 기존 패널 제거
    _Drawer->RemoveAllPanels();

    // 확인 패널
    Panel* confirmPanel = _Drawer->CreatePanel("Confirm", 35, 18, 80, 10);
    confirmPanel->SetBorder(true, ETextColor::WHITE);

    auto confirmText = std::make_unique<TextRenderer>();
    confirmText->AddLine("");
    confirmText->AddLine("     모험가 " + _PlayerName + "의 여정이 시작됩니다...");
    confirmText->AddLine("");
    confirmText->AddLine("     [아무 키나 눌러 계속...]");
    confirmText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    confirmPanel->SetContentRenderer(std::move(confirmText));

    _Drawer->Render();

    // 키 입력 대기
    while (!_Input->IsKeyPressed())
    {
        Sleep(50);
    }
    _Input->GetKeyCode();

    // 플레이어 생성 (인벤토리 활성화)
    Player* newPlayer = new Player(_PlayerName, true);  // ← enableInventory = true
    SceneManager::GetInstance()->SetPlayer(newPlayer);

    PrintManager::GetInstance()->PrintLogLine("플레이어 '" + _PlayerName + "' 생성 완료", ELogImportance::DISPLAY);

    // 다음 씬으로 전환
    _IsActive = false;
    Exit();

    // CharacterSelect 씬 전환
    SceneManager::GetInstance()->ChangeScene(ESceneType::CharacterSelect);
}
