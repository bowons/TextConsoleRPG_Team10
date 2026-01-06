#include "../../../include/UI/Scenes/CharacterSelectScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Common/TextColor.h"
#include <sstream>
#include <iomanip>
#include "../../../include/Unit/Player.h"

CharacterSelectScene::CharacterSelectScene()
    : UIScene("CharacterSelect")
    , _CurrentSelection(0)
{
}

CharacterSelectScene::~CharacterSelectScene()
{
}

void CharacterSelectScene::LoadClassData()
{
    _ClassDataList.clear();

    // CSV 파일 로드
    std::string charactersPath = DataManager::GetInstance()->GetResourcePath("Characters");
    auto csvData = DataManager::GetInstance()->LoadCSVFile(charactersPath, "Class.csv");

    // 헤더 스킵하고 데이터 파싱
    for (size_t i = 1; i < csvData.size(); ++i)
    {
        if (csvData[i].size() < 11) continue;

        ClassData data;
        data._ClassId = csvData[i][0];
        data._Role = csvData[i][1];
        data._RoleDesc = csvData[i][2];
        data._HP = std::stoi(csvData[i][3]);
        data._MP = std::stoi(csvData[i][4]);
        data._Atk = std::stoi(csvData[i][5]);
        data._Def = std::stoi(csvData[i][6]);
        data._Dex = std::stoi(csvData[i][7]);
        data._Luk = std::stoi(csvData[i][8]);
        data._CriticalRate = std::stof(csvData[i][9]);
        data._AsciiFile = csvData[i][10];
        data._AsciiFileSelect = csvData[i][11];

        _ClassDataList.push_back(data);
    }
}

void CharacterSelectScene::InitializeSkillData()
{
    _SkillDataList.clear();

    // Warrior 스킬
    ClassSkillData warrior;
    warrior._Skill1Name = "포효 (Roar)";
    warrior._Skill1Desc = "적들의 어그로를 자신에게 집중시키고 방어력 증가.";
    warrior._Skill2Name = "강타 (Smash)";
    warrior._Skill2Desc = "방어력의 일부를 공격력에 더해 물리 타격을 가함.";
    warrior._GrowthDesc = "전투 종료 시, 잃은 체력의 10% 확률로 최대 HP가 1~5 추가 상승.";
    _SkillDataList.push_back(warrior);

    // Mage 스킬
    ClassSkillData mage;
    mage._Skill1Name = "화염구 (Fireball)";
    mage._Skill1Desc = "단일 적에게 강력한 화염 피해.";
    mage._Skill2Name = "마력 폭주 (Overload)";
    mage._Skill2Desc = "다음 턴 마법 공격력을 2배로 높이지만 MP 소모량 증가.";
    mage._GrowthDesc = "전투 중 소모한 MP 총량에 비례하여 최대 MP 및 마법 공격력 상승.";
    _SkillDataList.push_back(mage);

    // Archer 스킬
    ClassSkillData archer;
    archer._Skill1Name = "정밀 조준 (Aim)";
    archer._Skill1Desc = "다음 공격의 치명타 확률을 대폭 상승.";
    archer._Skill2Name = "연사 (Multi-Shot)";
    archer._Skill2Desc = "적에게 3회 화살을 발사.";
    archer._GrowthDesc = "적을 처치하거나 치명타가 발생할 때마다 민첩성(DEX) 포인트 누적.";
    _SkillDataList.push_back(archer);

    // Priest 스킬
    ClassSkillData priest;
    priest._Skill1Name = "치유의 빛 (Heal)";
    priest._Skill1Desc = "아군 한 명의 체력을 대폭 회복.";
    priest._Skill2Name = "축복 (Bless)";
    priest._Skill2Desc = "일정 턴 동안 파티 전체의 공격력과 방어력을 상승.";
    priest._GrowthDesc = "아군에게 회복 및 버프를 성공시킨 횟수에 따라 회복 효율성 및 축복 버프 수치 강화.";
    _SkillDataList.push_back(priest);
}

void CharacterSelectScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;

    // 데이터 로드
    LoadClassData();
    InitializeSkillData();

    if (_ClassDataList.empty())
    {
        // 데이터 로드 실패 시 기본값
        _CurrentSelection = 0;
    }

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 배경 패널 (전체 화면) =====
    Panel* bgPanel = _Drawer->CreatePanel("Background", 0, 0, 150, 45);
    bgPanel->SetBorder(false);
    auto bgText = std::make_unique<TextRenderer>();
    bgText->SetTextColor(MakeColorAttribute(ETextColor::BLACK, EBackgroundColor::BLACK));
    bgPanel->SetContentRenderer(std::move(bgText));
    bgPanel->Redraw();

    // ===== 타이틀 패널 (중앙 상단) =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 45, 2, 60, 4);
    titlePanel->SetBorder(true, ETextColor::WHITE);

    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("");
    titleText->AddLineWithColor("                [ 직업 선택 ]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    titleText->AddLineWithColor("                 1/" + std::to_string(_ClassDataList.size()) + " 페이지",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    titlePanel->SetContentRenderer(std::move(titleText));
    titlePanel->Redraw();

    // ===== 이미지 패널 (왼쪽) =====
    Panel* imagePanel = _Drawer->CreatePanel("ClassImage", 10, 7, 65, 25);
    imagePanel->SetBorder(true, ETextColor::WHITE);

    auto dummyImage = std::make_unique<TextRenderer>();
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("");
    dummyImage->AddLine("                  [직업 별 초상화 이미지]");
    dummyImage->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    imagePanel->SetContentRenderer(std::move(dummyImage));
    imagePanel->Redraw();

    // ===== 오른쪽 정보 패널들 (세로로 분리) =====

    // 1. 직업 클래스명 패널
    Panel* classNamePanel = _Drawer->CreatePanel("ClassName", 77, 7, 63, 3);
    classNamePanel->SetBorder(true, ETextColor::WHITE);
    auto classNameText = std::make_unique<TextRenderer>();
    classNameText->AddLineWithColor("  직업 클래스명",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    classNamePanel->SetContentRenderer(std::move(classNameText));
    classNamePanel->Redraw();

    // 2. 역할 패널
    Panel* rolePanel = _Drawer->CreatePanel("Role", 77, 10, 63, 3);
    rolePanel->SetBorder(true, ETextColor::WHITE);
    auto roleText = std::make_unique<TextRenderer>();
    roleText->AddLineWithColor("  역할: ",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    rolePanel->SetContentRenderer(std::move(roleText));
    rolePanel->Redraw();

    // 3. 특징 패널
    Panel* descPanel = _Drawer->CreatePanel("Description", 77, 13, 63, 4);
    descPanel->SetBorder(true, ETextColor::WHITE);
    auto descText = std::make_unique<TextRenderer>();
    descText->AddLineWithColor("  특징:",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    descText->AddLine("  [직업 설명]");
    descPanel->SetContentRenderer(std::move(descText));
    descPanel->Redraw();

    // 4. 초기 스탯 패널
    Panel* statsPanel = _Drawer->CreatePanel("Stats", 77, 17, 63, 7);
    statsPanel->SetBorder(true, ETextColor::WHITE);
    auto statsText = std::make_unique<TextRenderer>();
    statsText->AddLineWithColor("  초기 스탯:",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    statsText->AddLine("  HP: -- | MP: --");
    statsText->AddLine("  ATK: -- | DEF: --");
    statsText->AddLine("  DEX: -- | LUK: --");
    statsText->AddLine("  CRIT: --%");
    statsPanel->SetContentRenderer(std::move(statsText));
    statsPanel->Redraw();

    // 5. 고유 스킬 패널
    Panel* skillsPanel = _Drawer->CreatePanel("Skills", 77, 24, 63, 4);
    skillsPanel->SetBorder(true, ETextColor::WHITE);
    auto skillsText = std::make_unique<TextRenderer>();
    skillsText->AddLineWithColor("  고유 스킬:",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    skillsText->AddLine("  • 스킬1");
    skillsText->AddLine("  • 스킬2");
    skillsPanel->SetContentRenderer(std::move(skillsText));
    skillsPanel->Redraw();

    // ===== 스킬 설명 패널 (중간) =====
    Panel* skillDescPanel = _Drawer->CreatePanel("SkillDesc", 10, 33, 130, 5);
    skillDescPanel->SetBorder(true, ETextColor::WHITE);

    auto skillDescText = std::make_unique<TextRenderer>();
    skillDescText->AddLineWithColor("  스킬 정보:",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    skillDescText->AddLine("  • 스킬1: 설명");
    skillDescText->AddLine("  • 스킬2: 설명");

    skillDescPanel->SetContentRenderer(std::move(skillDescText));
    skillDescPanel->Redraw();

    // ===== 안내 패널 (하단) =====
    Panel* guidePanel = _Drawer->CreatePanel("Guide", 10, 39, 130, 3);
    guidePanel->SetBorder(true, ETextColor::WHITE);

    auto guideText = std::make_unique<TextRenderer>();
    guideText->AddLineWithColor("  [ ←, →: 직업 선택 ] [ Enter: 확정 ] [ ESC: 이전 ]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    guidePanel->SetContentRenderer(std::move(guideText));
    guidePanel->Redraw();

    // 초기 데이터로 UI 업데이트
    if (!_ClassDataList.empty())
    {
        UpdateClassImage(_CurrentSelection);
        UpdateClassInfo(_CurrentSelection);
        UpdateGuideMessage(_CurrentSelection);
    }

    _Drawer->Render();
}

void CharacterSelectScene::UpdateClassImage(int selection)
{
    if (selection < 0 || selection >= static_cast<int>(_ClassDataList.size())) return;

    Panel* imagePanel = _Drawer->GetPanel("ClassImage");
    if (!imagePanel) return;

    auto artRenderer = std::make_unique<AsciiArtRenderer>();
    std::string resourcePath = DataManager::GetInstance()->GetResourcePath("Characters");

    // CSV에서 읽은 파일명 사용
    std::string fileName = _ClassDataList[selection]._AsciiFileSelect;

    // "Characters/" 접두사 제거
    if (fileName.find("Characters/") == 0)
    {
        fileName = fileName.substr(11);
    }

    // 아스키 아트 로드 시도
    bool loaded = artRenderer->LoadFromFile(resourcePath, fileName);

    if (loaded)
    {
        artRenderer->SetAlignment(ArtAlignment::CENTER);
        artRenderer->SetColor(ETextColor::LIGHT_CYAN);

        imagePanel->SetContentRenderer(std::move(artRenderer));
    }
    else
    {
        // 로드 실패 시 기본 텍스트 표시
        auto fallbackText = std::make_unique<TextRenderer>();
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLine("                  [이미지를 불러올 수 없습니다]");
        fallbackText->AddLine("");
        fallbackText->AddLine("                  파일: " + fileName);
        fallbackText->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

        imagePanel->SetContentRenderer(std::move(fallbackText));
    }

    imagePanel->Redraw();
}

void CharacterSelectScene::UpdateClassInfo(int selection)
{
    if (selection < 0 || selection >= static_cast<int>(_ClassDataList.size())) return;
    if (selection >= static_cast<int>(_SkillDataList.size())) return;

    const ClassData& classData = _ClassDataList[selection];
    const ClassSkillData& skillData = _SkillDataList[selection];

    // 1. 직업 클래스명 업데이트
    Panel* classNamePanel = _Drawer->GetPanel("ClassName");
    if (classNamePanel)
    {
        auto classNameText = std::make_unique<TextRenderer>();
        classNameText->AddLineWithColor("  " + classData._ClassId,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        classNamePanel->SetContentRenderer(std::move(classNameText));
        classNamePanel->Redraw();
    }

    // 2. 역할 업데이트
    Panel* rolePanel = _Drawer->GetPanel("Role");
    if (rolePanel)
    {
        auto roleText = std::make_unique<TextRenderer>();
        roleText->AddLineWithColor("  역할: " + classData._Role,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        rolePanel->SetContentRenderer(std::move(roleText));
        rolePanel->Redraw();
    }

    // 3. 특징 업데이트
    Panel* descPanel = _Drawer->GetPanel("Description");
    if (descPanel)
    {
        auto descText = std::make_unique<TextRenderer>();
        descText->AddLineWithColor("  특징:",
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        descText->AddLineWithColor("  " + classData._RoleDesc,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        descPanel->SetContentRenderer(std::move(descText));
        descPanel->Redraw();
    }

    // 4. 초기 스탯 업데이트
    Panel* statsPanel = _Drawer->GetPanel("Stats");
    if (statsPanel)
    {
        auto statsText = std::make_unique<TextRenderer>();
        statsText->AddLineWithColor("  초기 스탯:",
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        std::ostringstream oss;
        oss << "  HP: " << classData._HP << " | MP: " << classData._MP;
        statsText->AddLineWithColor(oss.str(),
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        oss.str("");
        oss << "  ATK: " << classData._Atk << " | DEF: " << classData._Def;
        statsText->AddLineWithColor(oss.str(),
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        oss.str("");
        oss << "  DEX: " << classData._Dex << " | LUK: " << classData._Luk;
        statsText->AddLineWithColor(oss.str(),
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        oss.str("");
        oss << std::fixed << std::setprecision(1);
        oss << "  CRIT: " << (classData._CriticalRate * 100) << "%";
        statsText->AddLineWithColor(oss.str(),
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        statsPanel->SetContentRenderer(std::move(statsText));
        statsPanel->Redraw();
    }

    // 5. 고유 스킬 업데이트
    Panel* skillsPanel = _Drawer->GetPanel("Skills");
    if (skillsPanel)
    {
        auto skillsText = std::make_unique<TextRenderer>();
        skillsText->AddLineWithColor("  고유 스킬:",
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        skillsText->AddLineWithColor("  • " + skillData._Skill1Name,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        skillsText->AddLineWithColor("  • " + skillData._Skill2Name,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        skillsPanel->SetContentRenderer(std::move(skillsText));
        skillsPanel->Redraw();
    }
}

void CharacterSelectScene::UpdateGuideMessage(int selection)
{
    if (selection < 0 || selection >= static_cast<int>(_ClassDataList.size())) return;
    if (selection >= static_cast<int>(_SkillDataList.size())) return;

    const ClassData& classData = _ClassDataList[selection];
    const ClassSkillData& skillData = _SkillDataList[selection];

    // 스킬 설명 패널 업데이트
    Panel* skillDescPanel = _Drawer->GetPanel("SkillDesc");
    if (skillDescPanel)
    {
        auto skillDescText = std::make_unique<TextRenderer>();
        skillDescText->AddLineWithColor("  스킬 정보:",
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        // 스킬 설명
        skillDescText->AddLineWithColor("  • " + skillData._Skill1Name + ": " + skillData._Skill1Desc,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        skillDescText->AddLineWithColor("  • " + skillData._Skill2Name + ": " + skillData._Skill2Desc,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        skillDescPanel->SetContentRenderer(std::move(skillDescText));
        skillDescPanel->Redraw();
    }
}

void CharacterSelectScene::UpdateTitlePage(int selection)
{
    Panel* titlePanel = _Drawer->GetPanel("Title");
    if (!titlePanel) return;

    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLine("");
    titleText->AddLineWithColor("                [ 직업 선택 ]",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    titleText->AddLineWithColor("                 " + std::to_string(selection + 1) + "/" + std::to_string(_ClassDataList.size()) + " 페이지",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    titlePanel->SetContentRenderer(std::move(titleText));
    titlePanel->Redraw();
}

void CharacterSelectScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void CharacterSelectScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void CharacterSelectScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void CharacterSelectScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;

    int keyCode = input->GetKeyCode();

    if (keyCode == 0x4B)  // 왼쪽 화살표
    {
        _CurrentSelection--;
        if (_CurrentSelection < 0) _CurrentSelection = static_cast<int>(_ClassDataList.size()) - 1;

        UpdateClassImage(_CurrentSelection);
        UpdateClassInfo(_CurrentSelection);
        UpdateGuideMessage(_CurrentSelection);
        UpdateTitlePage(_CurrentSelection);

        _Drawer->Render();
    }
    else if (keyCode == 0x4D)  // 오른쪽 화살표
    {
        _CurrentSelection++;
        if (_CurrentSelection >= static_cast<int>(_ClassDataList.size())) _CurrentSelection = 0;

        UpdateClassImage(_CurrentSelection);
        UpdateClassInfo(_CurrentSelection);
        UpdateGuideMessage(_CurrentSelection);
        UpdateTitlePage(_CurrentSelection);

        _Drawer->Render();
    }
    else if (keyCode == VK_RETURN || keyCode == '\r')  // Enter - 선택 확인
    {
        if (_CurrentSelection >= 0 &&
            _CurrentSelection < static_cast<int>(_ClassDataList.size()))
        {
            // 선택된 직업 데이터 가져오기
            const ClassData& selectedClass = _ClassDataList[_CurrentSelection];

            // GameManager에서 저장된 플레이어 이름 가져오기
            std::string playerName = GameManager::GetInstance()->GetTempPlayerName();

            // GameManager를 통해 CSV 기반 플레이어 생성
            GameManager* gm = GameManager::GetInstance();
            bool success = gm->CreateMainPlayerWithClass(playerName, selectedClass._ClassId);

            if (!success)
            {
                // 플레이어 생성 실패 시 에러 메시지 표시
                Panel* errorPanel = _Drawer->GetPanel("Guide");
                if (errorPanel)
                {
                    auto errorText = std::make_unique<TextRenderer>();
                    errorText->AddLineWithColor(
                        "  [ 플레이어 생성 실패! 다시 시도하세요. ]",
                        MakeColorAttribute(ETextColor::RED, EBackgroundColor::BLACK)
                    );
                    errorPanel->SetContentRenderer(std::move(errorText));
                    errorPanel->Redraw();
                    _Drawer->Render();
                }
                return;
            }

            // 다음 씬으로 이동 (StageSelect 또는 Battle)
            // SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
            // -> Floor 1에 대한 시나리오 출력
            SceneManager::GetInstance()->ChangeScene(ESceneType::StoryProgress);
        }
    }
    else if (keyCode == VK_ESCAPE)  // ESC - 이전 화면
    {
        SceneManager::GetInstance()->ChangeScene(ESceneType::PlayerNameInput);
    }
}
