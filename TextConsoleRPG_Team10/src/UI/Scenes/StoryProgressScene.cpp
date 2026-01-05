#include "../../../include/UI/Scenes/StoryProgressScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Common/TextColor.h"

StoryProgressScene::StoryProgressScene()
    : UIScene("StoryProgress")
    , _CurrentStoryIndex(0)
    , _TextComplete(false)
{
}

StoryProgressScene::~StoryProgressScene()
{
}

void StoryProgressScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 스토리 이미지 패널 (상단) =====
    Panel* imagePanel = _Drawer->CreatePanel("StoryImage", 10, 5, 130, 30);
    imagePanel->SetBorder(true, ETextColor::CYAN);

    // 더미 이미지 (플레이스홀더)
    auto dummyImage = std::make_unique<TextRenderer>();
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("   스토리 이미지");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    imagePanel->SetContentRenderer(std::move(dummyImage));
    imagePanel->Redraw();

    // TODO: 여기에서 스토리 이미지 조정
  // 스토리 진행 단계(_CurrentStoryIndex)에 따라 이미지를 변경합니다.
  //
    // 구현 방법:
    // Panel* imagePanel = _Drawer->GetPanel("StoryImage");
    // auto artRenderer = std::make_unique<AsciiArtRenderer>();
  // std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");
    //
    // // _CurrentStoryIndex에 따라 파일명 결정 (예: "Story1.txt", "Story2.txt")
    // std::string fileName = "Story" + std::to_string(_CurrentStoryIndex + 1) + ".txt";
    // if (artRenderer->LoadFromFile(uiPath, fileName)) {
    //artRenderer->SetAlignment(ArtAlignment::CENTER);
    //     artRenderer->SetColor(ETextColor::LIGHT_CYAN);
//     imagePanel->SetContentRenderer(std::move(artRenderer));
    //     imagePanel->Redraw();
    // }

    // ===== 스토리 텍스트 패널 (하단) =====
    Panel* textPanel = _Drawer->CreatePanel("StoryText", 25, 36, 100, 8);
    textPanel->SetBorder(true, ETextColor::LIGHT_CYAN);

    auto storyText = std::make_unique<TextRenderer>();
    storyText->AddLine("");
    storyText->AddLineWithColor("    스토리 텍스트",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    storyText->AddLine("");
    storyText->AddLine("  → 1~2 문장씩 출력하여 출력되게끔");
    storyText->AddLine("");

    textPanel->SetContentRenderer(std::move(storyText));
    textPanel->Redraw();

    // TODO: 여기에서 스토리 텍스트 동적 업데이트
        // 스토리 진행 단계에 따라 텍스트를 변경하고, 타이핑 효과를 적용합니다.
        //
        // 구현 방법:
     //Panel* textPanel = _Drawer->GetPanel("StoryText");
     //auto storyText = std::make_unique<TextRenderer>();
        //
        // // _CurrentStoryIndex에 따라 스토리 텍스트 표시
        // storyText->AddLine("");
        // storyText->AddLineWithTyping("어둠의 탑 '에레보스'가 등장했다...", 
        //   MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        //
    // 타이핑 효과 활성화
    storyText->EnableTypingEffect(true);
    storyText->SetTypingSpeed(ETypingSpeed::Normal);
    
     textPanel->SetContentRenderer(std::move(storyText));
     textPanel->Redraw();
        //
        // // CSV 파일에서 스토리 로드:
        // // DataManager::GetInstance()->LoadTextFile(경로, "Story.txt");
        // // 또는 LoadCSVFile()로 여러 스토리 단계 관리

    _Drawer->Render();
}

void StoryProgressScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void StoryProgressScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void StoryProgressScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void StoryProgressScene::HandleInput()
{
    
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;
    int keyCode = input->GetKeyCode();
    
     if (keyCode == VK_SPACE || keyCode == VK_RETURN) {  // Space or Enter
       // 타이핑 효과가 진행 중이면 즉시 완료
         if (!_TextComplete) {
           _TextComplete = true;
             // 텍스트 즉시 표시
             return;
         }
    
         // 다음 스토리 단계로
       _CurrentStoryIndex++;
    
         // 스토리 끝났으면 다음 씬으로
     //    if (_CurrentStoryIndex >= 총스토리수) {
     //SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
     //    }
     //    else {
     //        // 이미지 패널 업데이트 (위의 TODO 참고)
     // // 텍스트 패널 업데이트 (위의 TODO 참고)
     //        _TextComplete = false;
     //     _Drawer->Render();
     //    }
     }
     else if (keyCode == VK_ESCAPE) {  // ESC - 스토리 스킵
         SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
     }
}
