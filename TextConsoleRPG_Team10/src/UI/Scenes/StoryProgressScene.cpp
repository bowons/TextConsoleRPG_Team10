#include "../../../include/UI/Scenes/StoryProgressScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/StageManager.h"
#include "../../../include/Manager/SoundPlayer.h"
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
    
    // 출력할 스토리 텍스트 가져오기
    GetStoriesData();
    // BGM 변경
    SoundPlayer::GetInstance()->PlayBGM(_BGMID);

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 스토리 이미지 패널 (상단) =====
    Panel* ImagePanel = _Drawer->CreatePanel("StoryImage", 10, 5, 130, 30);
    ImagePanel->SetBorder(true, ETextColor::CYAN);

    //// 더미 이미지 (플레이스홀더)
    auto DummyImage = std::make_unique<TextRenderer>();
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("   스토리 이미지");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->AddLine("");
    DummyImage->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    ImagePanel->SetContentRenderer(std::move(DummyImage));
    ImagePanel->Redraw();

    // TODO: 여기에서 스토리 이미지 조정
  // 스토리 진행 단계(_CurrentStoryIndex)에 따라 이미지를 변경합니다.
  //
  //   구현 방법:
     //Panel* imagePanel = _Drawer->GetPanel("StoryImage");
    auto ArtRenderer = std::make_unique<AsciiArtRenderer>();
    std::string UIPath = DataManager::GetInstance()->GetResourcePath("UI");
    
     // _CurrentStoryIndex에 따라 파일명 결정 (예: "Story1.txt", "Story2.txt")
     //std::string fileName = "Story" + std::to_string(_CurrentStoryIndex + 1) + ".txt";
    std::string FileName = "Sewer.txt";
    if (ArtRenderer->LoadFromFile(UIPath, FileName)) {
        ArtRenderer->SetAlignment(ArtAlignment::CENTER);
        ArtRenderer->SetColor(ETextColor::LIGHT_CYAN);
        ImagePanel->SetContentRenderer(std::move(ArtRenderer));
        ImagePanel->Redraw();
     }

    // ===== 스토리 텍스트 패널 (하단) =====
    Panel* TextPanel = _Drawer->CreatePanel("StoryText", 25, 36, 100, 8);
    TextPanel->SetBorder(true, ETextColor::LIGHT_CYAN);

    auto StoryText = std::make_unique<TextRenderer>();
   /* StoryText->AddLine("");
    StoryText->AddLineWithColor("    스토리 텍스트",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    StoryText->AddLine("");
    StoryText->AddLine("  → 1~2 문장씩 출력하여 출력되게끔");
    StoryText->AddLine("");*/

    // textPanel->SetContentRenderer(std::move(storyText));
    // textPanel->Redraw();

    // TODO: 여기에서 스토리 텍스트 동적 업데이트
        // 스토리 진행 단계에 따라 텍스트를 변경하고, 타이핑 효과를 적용합니다.
        //
        // 구현 방법:
    // Panel* textPanel = _Drawer->GetPanel("StoryText");
    // auto storyText = std::make_unique<TextRenderer>();
        
    // _CurrentStoryIndex에 따라 스토리 텍스트 표시
    // storyText->AddLine("");
    // storyText->AddLineWithTyping("어둠의 탑 '에레보스'가 등장했다...", 
    // MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        
    // 타이핑 효과 활성화
    StoryText->EnableTypingEffect(true);
    StoryText->SetTypingSpeed(ETypingSpeed::Normal);
    
    // 플로어 설명 추가
    StoryText->AddLineWithTyping(_StoryTexts[0][4],
        static_cast<int>(ETextColor::LIGHT_YELLOW));

    // 파티 여부에 따른 텍스트 추가
    const auto& Party = GameManager::GetInstance()->GetParty();
    if (Party.size() > 1)
    {
        StoryText->AddLineWithTyping(_StoryTexts[1][4]);
    }
    else
    {
        StoryText->AddLineWithTyping(_StoryTexts[2][4]);
    }
    StoryText->AddLineWithTyping(_StoryTexts[3][4]);

    // Todo 파티(플레이어 제외) 내 직업에 따른 텍스트 출력


    TextPanel->SetContentRenderer(std::move(StoryText));
    TextPanel->Redraw();
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

    // 데이터 리셋
    _StoryTexts.clear();
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

void StoryProgressScene::GetStoriesData()
{
    if (DataManager::GetInstance())
    {
        const auto& Datas = DataManager::GetInstance()->LoadCSVFile(
            DataManager::GetInstance()->GetResourcePath("Stories"),
            "Stories.csv");
        GetFloorData(StageManager::GetInstance()->GetCurrentFloor());

        for (auto Row : Datas)
        {
            if (Row[1] == _FloorName)
            {
                _StoryTexts.push_back(Row);
            }
        }
    }
}

void StoryProgressScene::GetFloorData(int FloorIndex)
{
    switch (FloorIndex)
    {
        case 1:
        {
            _FloorName = "Sewer";
            _BGMID = "BGM_Sewer";
        }
        break;
        case 2:
        {
            _FloorName = "Mine";
            _BGMID = "BGM_Mine";
        }
        break;
        case 3:
        {
            _FloorName = "Grave";
            _BGMID = "BGM_Grave";
        }
        break;
        case 4:
        {
            _FloorName = "Ruin";
            _BGMID = "BGM_Ruin";
        }
        break;
        case 5:
        {
            _FloorName = "Nest";
            _BGMID = "BGM_Nest";
        }
        break;
        case 6:
        {
            _FloorName = "SnowField";
            _BGMID = "BGM_SnowField";
        }
        break;
        case 7:
        {
            _FloorName = "BloodyMoon";
            _BGMID = "BGM_BloodyMoon";
        }
        break;
        case 8:
        {
            _FloorName = "Fog";
            _BGMID = "BGM_Fog";
        }
        break;
        case 9:
        {
            _FloorName = "Altar";
            _BGMID = "BGM_Altar";
        }
        break;
        case 10:
        {
            _FloorName = "Void";
            _BGMID = "BGM_Void";
        }
        break;
        default:
        {
            _FloorName = "Start";
            _BGMID = "BGM_Start";
        }
    }
}