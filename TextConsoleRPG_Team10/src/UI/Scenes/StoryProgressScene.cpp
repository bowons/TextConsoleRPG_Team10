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
#include "../../../include/Unit/Player.h"
#include "../../../include/Unit/Warrior.h"
#include "../../../include/Unit/Mage.h"
#include "../../../include/Unit/Archer.h"
#include "../../../include/Unit/Priest.h"
#include <memory>

StoryProgressScene::StoryProgressScene()
    : UIScene("StoryProgress")
    , _CurrentStoryIndex(0)
    , _TextComplete(false)
    , _IsFirst(true)
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
    _Floor = StageManager::GetInstance()->GetCurrentFloor();

    // 첫 시작 시 시나리오 출력을 위한 예외 처리
    if (_IsFirst) _Floor = 0;
    else if (GameManager::GetInstance()->GetMainPlayer().get()->IsDead())
    {
        _Floor = 11;
    }
    GetStoriesData(_Floor);

    // BGM 변경
    std::string BGMID = "BGM_" + std::to_string(_Floor);
    SoundPlayer::GetInstance()->PlayBGM(BGMID);

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
    std::string FileName = "Story" + std::to_string(_Floor) + ".txt";
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
    //StoryText->AddLine("");
    //StoryText->AddLineWithColor("    스토리 텍스트",
    //    MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    //StoryText->AddLine("");
    //StoryText->AddLine("  → 1~2 문장씩 출력하여 출력되게끔");
    //StoryText->AddLine("");

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
    
    // 자동 줄바꿈 활성화 및 너비 설정
    StoryText->EnableAutoWrap(true);
    StoryText->SetWrapWidth(96);
    // 타이핑 효과 활성화
    StoryText->EnableTypingEffect(true);
    StoryText->SetTypingSpeed(ETypingSpeed::Normal);
    TextPanel->SetContentRenderer(std::move(StoryText));
    UpdateUIWithCurrentStory();
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
    _CurrentStoryIndex = 0;
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

void StoryProgressScene::HandleInput() {
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;
    int keyCode = input->GetKeyCode();

    if (keyCode == VK_SPACE || keyCode == VK_RETURN) {
        Panel* panel = _Drawer->GetPanel("StoryText");
        auto* renderer = static_cast<TextRenderer*>(panel->GetContentRenderer());

        // 1. 타이핑 중이면 스킵
        if (!renderer->IsTypingFinished()) {
            renderer->SkipTyping();
        }
        // 2. 타이핑이 끝났으면 다음 행으로
        else {
            _CurrentStoryIndex++;
            if (_CurrentStoryIndex < _StoryTexts.size()) {
                UpdateUIWithCurrentStory();

                if (_Floor == 0)
                {
                    CheckArtUpdate();
                }
            }
            else {
                // 더 이상 보여줄 데이터가 없으면 다음 씬으로
                if (!_IsFirst)
                {
                    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
                }
                else
                {
                    _IsFirst = false;
                    SceneManager::GetInstance()->ChangeScene(ESceneType::PlayerNameInput);
                }
            }
        }
        _Drawer->Render();
    }
    else if (keyCode == VK_ESCAPE) {  // ESC - 스토리 스킵
        if (_IsFirst)
        {
            _IsFirst = false;
            SceneManager::GetInstance()->ChangeScene(ESceneType::PlayerNameInput);
        }
        else if(GameManager::GetInstance()->GetMainPlayer().get()->IsDead())
        {
            SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
        }
        else
        {
            SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
        }
    }
}

void StoryProgressScene::ResetIsFirst()
{
    _IsFirst = true;
}

void StoryProgressScene::UpdateUIWithCurrentStory()
{
    if (_CurrentStoryIndex >= _StoryTexts.size()) return;

    // 패널에서 렌더러 꺼내오기
    Panel* Panel = _Drawer->GetPanel("StoryText");
    auto* Renderer = static_cast<TextRenderer*>(Panel->GetContentRenderer());

    Renderer->AddLineWithTyping(_StoryTexts[_CurrentStoryIndex],
        _LineColor[_CurrentStoryIndex]);
}

// 아예 데이터를 가져오는 단계에서 걸러서 가져오기
void StoryProgressScene::GetStoriesData(int FloorIndex)
{
    if (DataManager::GetInstance())
    {
        const auto& Datas = DataManager::GetInstance()->LoadCSVFile(
            DataManager::GetInstance()->GetResourcePath("Stories"),
            "Stories.csv");
        // 현재 파티 내부의 직업 및 그 이름 체크
        // 0 : Warrior 이름들, 1 : Mage 이름들, 2 : Archer 이름들, 3 : Priest 이름들
        std::vector<std::vector<std::string>> NamePerJob(4, std::vector<std::string>());
        // Todo - GameManager::GetInstance()->GetParty()와 dynamic_cast를 사용해 분류하기
        CheckPartyInfo(NamePerJob);

        // 0번째는 Column 이름
        for (int i = 1; i < Datas.size(); ++i)
        {
            // CSV 구조: 0:ID, 1:Floor, 2:Type, 3:Speaker, 4:Content
            if (stoi(Datas[i][1]) == FloorIndex)
            {
                if (Datas[i][2] == "Solo" && GameManager::GetInstance()->GetPartySize() > 1) continue;
                else if (Datas[i][2] == "Party" && GameManager::GetInstance()->GetPartySize() <= 1) continue;
                else if (Datas[i][3] == "Warrior" && NamePerJob[0].size() == 0) continue;
                else if (Datas[i][3] == "Mage" && NamePerJob[1].size() == 0) continue;
                else if (Datas[i][3] == "Archer" && NamePerJob[2].size() == 0) continue;
                else if (Datas[i][3] == "Priest" && NamePerJob[3].size() == 0) continue;

               
                const std::string& Speaker = Datas[i][3];
                const std::string& Content = Datas[i][4];

                // 화자 이름 포맷팅 (예: [System] : ...)
                std::string Prefix = "";
                if (Speaker == "Player" && GameManager::GetInstance()->GetPartySize() > 0)
                {
                    Prefix = "[" + GameManager::GetInstance()->GetParty()[0].get()->GetName() + "]: ";
                }
                else if (Speaker == "Warrior" && NamePerJob[0].size() > 0)
                {
                    int Index = rand() % NamePerJob[0].size();
                    Prefix = "[" + NamePerJob[0][Index] + "] : ";
                }
                else if (Speaker == "Mage" && NamePerJob[1].size() > 0)
                {
                    int Index = rand() % NamePerJob[1].size();
                    Prefix = "[" + NamePerJob[1][Index] + "] : ";
                }
                else if (Speaker == "Archer" && NamePerJob[2].size() > 0)
                {
                    int Index = rand() % NamePerJob[2].size();
                    Prefix = "[" + NamePerJob[2][Index] + "] : ";
                }
                else if (Speaker == "Priest" && NamePerJob[3].size() > 0)
                {
                    int Index = rand() % NamePerJob[3].size();
                    Prefix = "[" + NamePerJob[3][Index] + "] : ";
                }

                // 문자열 분리
                std::vector<std::string> Lines;
                SplitText(Lines, Content, "\\n");
                for (std::string& Line : Lines)
                {
                    Line = Prefix + Line;
                }

                for (const std::string& Line : Lines)
                {
                    if (Speaker == "System")
                    {
                        _LineColor.push_back(14);
                    }
                    else
                    {
                        _LineColor.push_back(15);
                    }
                    _StoryTexts.push_back(Line);
                }
            }
        }
    }
}

void StoryProgressScene::SplitText(std::vector<std::string>& OutList, const std::string& Content, const std::string& Delimiter)
{
    OutList.clear();
    if (Content.empty()) return;

    std::string temp = "";
    for (size_t i = 0; i < Content.length(); ++i)
    {
        // 1. 구분자(\\n) 검색: 역슬래시(\)를 만났을 때 다음 문자가 n인지 확인
        if (Content[i] == '\\' && i + 1 < Content.length() && Content[i + 1] == 'n')
        {
            OutList.push_back(temp);
            temp.clear();
            i++; // 'n' 위치까지 점프
            continue;
        }

        // 2. 한글 및 멀티바이트 보호
        // 바이트 값이 128(0x80) 이상이면 한글의 일부입니다.
        if (static_cast<unsigned char>(Content[i]) >= 0x80)
        {
            temp += Content[i]; // 첫 바이트 저장
            if (i + 1 < Content.length())
            {
                temp += Content[++i]; // 다음 바이트까지 세트로 저장 (한글 보존)
            }
        }
        else
        {
            temp += Content[i]; // 영문, 숫자, 기호
        }
    }
    if (!temp.empty()) OutList.push_back(temp);
}

void  StoryProgressScene::CheckPartyInfo(std::vector<std::vector<std::string>>& OutNamePerJob)
{
    // 파티 정보 분석
    const auto& PartyList = GameManager::GetInstance()->GetParty();
    bool IsSolo = GameManager::GetInstance()->GetPartySize() == 1;

    // 직업별 이름 저장용 (Warrior: 0, Mage: 1, Archer: 2, Priest: 3)
    std::vector<std::vector<std::string>> NamePerJob(4);

    for (size_t i = 1; i < PartyList.size(); i++) // 0번 Player 제외
    {
        if (dynamic_cast<Warrior*>(PartyList[i].get())) NamePerJob[0].push_back(PartyList[i]->GetName());
        else if (dynamic_cast<Mage*>(PartyList[i].get())) NamePerJob[1].push_back(PartyList[i]->GetName());
        else if (dynamic_cast<Archer*>(PartyList[i].get())) NamePerJob[2].push_back(PartyList[i]->GetName());
        else if (dynamic_cast<Priest*>(PartyList[i].get())) NamePerJob[3].push_back(PartyList[i]->GetName());
    }
}

void StoryProgressScene::SetArtImage(std::string FileName)
{
    if (Panel* Panel = _Drawer->GetPanel("StoryImage"))
    {
        if (auto* Renderer = static_cast<AsciiArtRenderer*>(Panel->GetContentRenderer()))
        {
            std::string UIPath = DataManager::GetInstance()->GetResourcePath("UI");
            if (Renderer->LoadFromFile(UIPath, FileName)) {
                Renderer->SetAlignment(ArtAlignment::CENTER);
                Renderer->SetColor(ETextColor::LIGHT_CYAN);
                Panel->Redraw();
            }
        }
    }
}

void StoryProgressScene::CheckArtUpdate()
{
    std::string FileName = "Story" + std::to_string(_Floor);
    if (_Floor == 0)
    {
        if (_CurrentStoryIndex == 8)
        {
            FileName.append("_2");
        }
        else if (_CurrentStoryIndex == 14)
        {
            FileName.append("_3");
        }
        else if (_CurrentStoryIndex == 24)
        {
            FileName.append("_4");
        }
        else
        {
            return;
        }
    }
    FileName.append(".txt");
    SetArtImage(FileName);
}

