#include "../../../include/UI/Scenes/CompanionRecruitScene.h"

#include <Windows.h>

#include "../../../include/Common/TextColor.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/StageManager.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/Unit/Player.h"
#include "../../../include/Unit/Warrior.h"
#include "../../../include/Unit/Mage.h"
#include "../../../include/Unit/Archer.h"
#include "../../../include/Unit/Priest.h"
#include "../../../include/Item/Inventory.h"
#include "../../../include/Item/IItem.h"
#include "../../../include/Data/ClassData.h"

CompanionRecruitScene::CompanionRecruitScene()
    : UIScene("CompanionRecruit"),
      _CompanionLevel(1),
      _InputName(""),
      _RecruitAccepted(false),
      _SelectedOption(0) {}

CompanionRecruitScene::~CompanionRecruitScene() {}

void CompanionRecruitScene::Enter() {
  _Drawer->ClearScreen();
  _Drawer->RemoveAllPanels();
  _Drawer->Activate();
  _IsActive = true;
  _RecruitAccepted = false;
  _SelectedOption = 0;
  _InputName.clear();

  // 입력 버퍼 클리어
  HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
  FlushConsoleInputBuffer(hInput);

  StageManager* stageMgr = StageManager::GetInstance();
  const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();
  
  // 현재 층수에 따라 동료 레벨 결정
  _CompanionLevel = floorInfo->Floor;
  
  // 랜덤 동료 선택
  DataManager* dataMgr = DataManager::GetInstance();
  _CurrentCompanion = dataMgr->GetRandomCompanion();
  
  if (!_CurrentCompanion.has_value())
  {
      // 동료 데이터 로드 실패 시 StageSelect로 복귀
      _IsActive = false;
      SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
      return;
  }

  // =============================================================================
  // 패널 레이아웃 (150x45 화면 기준)
  // =============================================================================

  // ===== 타이틀 패널 (상단) =====
  Panel* titlePanel = _Drawer->CreatePanel("Title", 0, 0, 122, 5);
  titlePanel->SetBorder(true, ETextColor::WHITE);

  auto titleText = std::make_unique<TextRenderer>();
  titleText->AddLine("");
  titleText->AddLineWithColor(
      "                    [ 동료 영입 ] - " + std::to_string(_CompanionLevel) + "층",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  titleText->AddLine("");

  titlePanel->SetContentRenderer(std::move(titleText));
  titlePanel->Redraw();

  // ===== 장소 이미지 패널 (좌측 중앙) =====
  Panel* locationPanel = _Drawer->CreatePanel("Location", 0, 5, 61, 24);
  locationPanel->SetBorder(true, ETextColor::WHITE);

  auto locationArt = std::make_unique<AsciiArtRenderer>();
  std::string mapsPath = DataManager::GetInstance()->GetResourcePath("Maps");
  locationArt->LoadFromFile(mapsPath, "Prison_Entrance.txt");

  locationPanel->SetContentRenderer(std::move(locationArt));
  locationPanel->Redraw();

  // ===== 동료 캐릭터 이미지 + 정보 패널 (우측 중앙) =====
  Panel* characterPanel = _Drawer->CreatePanel("Character", 61, 5, 61, 24);
  characterPanel->SetBorder(true, ETextColor::WHITE);

  UpdateCompanionInfoPanel(characterPanel);

  // ===== 타워(맵) 패널 (우측 상단) =====
  Panel* towerPanel = _Drawer->CreatePanel("Tower", 122, 1, 30, 30);
  auto towerArt = std::make_unique<AsciiArtRenderer>();
  std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");

  bool towerLoaded = towerArt->LoadFromFile(uiPath, "Tower.txt");

  if (towerLoaded) {
    towerArt->SetAlignment(ArtAlignment::CENTER);
    towerArt->SetColor(ETextColor::WHITE);
    towerPanel->SetContentRenderer(std::move(towerArt));
  } else {
    auto errorText = std::make_unique<TextRenderer>();
    errorText->AddLine("");
    errorText->AddLine("");
    errorText->AddLineWithColor("[ Tower.txt not found ]",
                                static_cast<WORD>(ETextColor::WHITE));
    errorText->AddLine("");
    errorText->SetTextColor(static_cast<WORD>(ETextColor::WHITE));
    towerPanel->SetContentRenderer(std::move(errorText));
  }

  // 타워 화살표 업데이트
  UpdateTowerArrow(towerPanel, floorInfo->Floor);

  // ===== 아군 파티 패널 (하단 상단, 4명 가로 배치) =====
  auto party = GameManager::GetInstance()->GetParty();

  int partyStartX = 0;
  int partyStartY = 29;
  int partyHeight = 7;
  int partyWidth = 30;

  for (int i = 0; i < 4; ++i) {
    int xPos = partyStartX + i * partyWidth;
    std::string panelName = "Party" + std::to_string(i);

    Panel* partyPanel = _Drawer->CreatePanel(panelName, xPos, partyStartY,
                                             partyWidth, partyHeight);
    partyPanel->SetBorder(true, ETextColor::WHITE);

    auto partyText = std::make_unique<TextRenderer>();
    partyText->AddLine("");

    if (i < party.size() && party[i]) {
      Player* player = party[i].get();
      std::string name = player->GetName();
      std::string className = "전사";  // TODO: 직업 추가
      int hp = player->GetCurrentHP();
      int maxHp = player->GetMaxHP();
      int atk = player->GetAtk();
      int def = player->GetDef();

      partyText->AddLine(" " + name + " | " + className);
      partyText->AddLine(" HP:" + std::to_string(hp) + "/" +
                         std::to_string(maxHp));
      partyText->AddLine(" ATK:" + std::to_string(atk) +
                         " DEF:" + std::to_string(def));
    } else {
      partyText->AddLine(" 빈 슬롯");
      partyText->AddLine("");
    }

    partyPanel->SetContentRenderer(std::move(partyText));
    partyPanel->Redraw();
  }

  // ===== 시스템 로그 패널 (하단 좌측-중앙) =====
  Panel* logPanel = _Drawer->CreatePanel("SystemLog", 0, 36, 113, 9);
  logPanel->SetBorder(true, ETextColor::WHITE);

  // 좌측 영역: 시스템 로그 (1 ~ 74)
  auto logText = std::make_unique<TextRenderer>();
  logText->AddLine("");
  logText->AddLineWithColor(
      "  [ 시스템 로그 ]",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  logText->AddLine("");
  logText->AddLine("  " + _CurrentCompanion->_Name + "을(를) 발견했습니다!");
  logText->AddLine("  영입하시겠습니까?");
  logPanel->AddRenderer(1, 0, 74, 10, std::move(logText));

  // 우측 영역: 커맨드 (75 ~ 111)
  auto logCommandText = std::make_unique<TextRenderer>();
  logCommandText->AddLine("");
  logCommandText->AddLineWithColor(
      "  [ 커맨드 ]",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  logCommandText->AddLine("");
  logCommandText->AddLine("  [← /→ ] 선택");
  logCommandText->AddLine("  [Enter] 확인");
  logCommandText->AddLine("  [ESC] 거부");
  logPanel->AddRenderer(75, 0, 37, 10, std::move(logCommandText));

  logPanel->Redraw();

  // ===== 인벤토리 & 골드 패널 (하단 우측) =====
  Panel* inventoryCommandPanel =
      _Drawer->CreatePanel("Command", 113, 36, 37, 9);
  inventoryCommandPanel->SetBorder(true, ETextColor::WHITE);

  auto inventoryCommandText = std::make_unique<TextRenderer>();
  inventoryCommandText->AddLine("");
  inventoryCommandText->AddLineWithColor(
      "  인벤토리 & 골드",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  inventoryCommandText->AddLine("");

  if (!party.empty() && party[0]) {
    Inventory* inventory = nullptr;
    if (party[0]->TryGetInventory(inventory) && inventory) {
      int usedSlots = 0;
      const int maxSlots = 5;
      for (int i = 0; i < maxSlots; ++i) {
        if (inventory->GetItemAtSlot(i) != nullptr)
          usedSlots++;
      }
      inventoryCommandText->AddLine("  인벤토리: " + std::to_string(usedSlots) + "/" + std::to_string(maxSlots));
    }
    inventoryCommandText->AddLine("  골드: " + std::to_string(party[0]->GetGold()) + " G");
  }

  inventoryCommandPanel->SetContentRenderer(std::move(inventoryCommandText));
  inventoryCommandPanel->Redraw();

  _Drawer->Render();
}

void CompanionRecruitScene::UpdateCompanionInfoPanel(Panel* infoPanel) {
  if (!infoPanel || !_CurrentCompanion.has_value()) return;

  infoPanel->ClearRenderers();

  const CompanionData& companion = _CurrentCompanion.value();

  // 상단: 아스키 아트 (0 ~ 15줄)
  auto characterArt = std::make_unique<AsciiArtRenderer>();
  std::string charactersPath = DataManager::GetInstance()->GetResourcePath("Characters");
  
  // CSV에서 읽은 파일명 사용
  std::string fileName = companion._AsciiFile;

  // "Characters/" 접두사 제거
  if (fileName.find("Characters/") == 0)
  {
      fileName = fileName.substr(11);
  }
  
  bool artLoaded = characterArt->LoadFromFile(charactersPath, fileName);
  
  if (artLoaded) {
    characterArt->SetAlignment(ArtAlignment::CENTER);
    infoPanel->AddRenderer(0, 0, 59, 15, std::move(characterArt));
  } else {
    // 로드 실패 시 에러 메시지 표시
    auto errorText = std::make_unique<TextRenderer>();
    errorText->AddLine("");
    errorText->AddLineWithColor(
        "  [ 캐릭터 이미지 로드 실패 ]",
        MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
    errorText->AddLine("");
    errorText->AddLine("  경로: " + charactersPath);
    errorText->AddLine("  파일: " + fileName);
    infoPanel->AddRenderer(0, 0, 59, 15, std::move(errorText));
  }

  // 하단: 동료 정보 (16줄부터)
  auto infoText = std::make_unique<TextRenderer>();
  infoText->AddLine("");
  infoText->AddLineWithColor(
      "  [ " + companion._Name + " ]",
      MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
  infoText->AddLine("");

  // 직업 표시
  std::string jobName;
  if (companion._JobType == "warrior") jobName = "전사";
  else if (companion._JobType == "mage") jobName = "마법사";
  else if (companion._JobType == "archer") jobName = "궁수";
  else if (companion._JobType == "priest") jobName = "사제";
  else jobName = companion._JobType;

  infoText->AddLine("  직업: " + jobName);
  infoText->AddLine("  레벨: " + std::to_string(_CompanionLevel));
  infoText->AddLine("");

  // 스탯 표시 (레벨 보정 적용)
  int levelBonus = (_CompanionLevel - 1);
  int adjustedHP = companion._HP + (levelBonus * 20);
  int adjustedATK = companion._Atk + (levelBonus * 5);

  infoText->AddLine("  HP: " + std::to_string(adjustedHP));
  infoText->AddLine("  MP: " + std::to_string(companion._MP));
  infoText->AddLine("  ATK: " + std::to_string(adjustedATK) + " / DEF: " + std::to_string(companion._Def));
  infoText->AddLine("  DEX: " + std::to_string(companion._Dex) + " / LUK: " + std::to_string(companion._Luk));
  infoText->AddLine("  치명타율: " + std::to_string(static_cast<int>(companion._CritRate * 100)) + "%");

  infoPanel->AddRenderer(0, 16, 59, 8, std::move(infoText));
  infoPanel->Redraw();
}

void CompanionRecruitScene::UpdateTowerArrow(Panel* towerPanel, int currentFloor) {
  auto arrowRenderer = std::make_unique<TextRenderer>();

  const int towerHeight = 25;
  const int maxFloor = 10;
  const int topMargin = 6;
  const int bottomMargin = 0;
  const int usableHeight = towerHeight - topMargin - bottomMargin;

  int arrowLine = topMargin + ((maxFloor - currentFloor) * usableHeight / maxFloor);

  for (int i = 0; i < arrowLine; ++i) {
    arrowRenderer->AddLine("");
  }

  arrowRenderer->AddLineWithColor("*----►",
      MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

  towerPanel->AddRenderer(0, 0, 5, towerHeight, std::move(arrowRenderer));
  towerPanel->Redraw();
}

void CompanionRecruitScene::Exit() {
  _Drawer->RemoveAllPanels();
  _IsActive = false;
  _CurrentCompanion.reset();
}

void CompanionRecruitScene::Update() {
  if (_IsActive) {
    _Drawer->Update();
    HandleInput();
  }
}

void CompanionRecruitScene::Render() {
  // UIDrawer::Update()에서 자동 렌더링
}

void CompanionRecruitScene::HandleInput() {
  InputManager* input = InputManager::GetInstance();
  if (!input->IsKeyPressed()) return;

  int keyCode = input->GetKeyCode();
  if (keyCode == 0 || keyCode < 0) return;

  // ESC: 거부 (StageSelect로 복귀)
  if (keyCode == VK_ESCAPE) {
    _RecruitAccepted = false;
    _IsActive = false;
    Exit();
    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
    return;
  }

  // 좌우 방향키: 선택 전환
  if (keyCode == VK_LEFT || keyCode == 75) {  // 왼쪽 화살표
    _SelectedOption = 0;  // 영입
    UpdateSelectionUI();
  } else if (keyCode == VK_RIGHT || keyCode == 77) {  // 오른쪽 화살표
    _SelectedOption = 1;  // 거부
    UpdateSelectionUI();
  }

  // Enter: 선택 확인
  if (keyCode == VK_RETURN) {
    if (_SelectedOption == 0 && _CurrentCompanion.has_value()) {
      // 영입 수락
      _RecruitAccepted = true;
      
      // 동료를 파티에 추가
      GameManager* gm = GameManager::GetInstance();
      auto& party = const_cast<std::vector<std::shared_ptr<Player>>&>(gm->GetParty());
      
      // 파티 인원 확인 (최대 4명)
      if (party.size() >= 4) {
        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) {
          logPanel->ClearRenderers();
          auto errorText = std::make_unique<TextRenderer>();
          errorText->AddLine("");
          errorText->AddLineWithColor(
              "  [ 시스템 로그 ]",
              MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
          errorText->AddLine("");
          errorText->AddLineWithColor(
              "  파티가 가득 찼습니다!",
              MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
          errorText->AddLine("  (Enter를 눌러 계속)");
          logPanel->AddRenderer(1, 0, 74, 10, std::move(errorText));
          logPanel->Redraw();
          _Drawer->Render();
          
          Sleep(1500);
          _IsActive = false;
          Exit();
          SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
          return;
        }
      }
      
      const CompanionData& companion = _CurrentCompanion.value();
      
      // ClassData 구조체 생성 (CSV 기반 생성자용)
      ClassData classData;
      classData._ClassId = companion._JobType;
      
      // 레벨 보정 적용
      int levelBonus = (_CompanionLevel - 1);
      classData._HP = companion._HP + (levelBonus * 20);
      classData._MP = companion._MP;
      classData._Atk = companion._Atk + (levelBonus * 5);
      classData._Def = companion._Def;
      classData._Dex = companion._Dex;
      classData._Luk = companion._Luk;
      classData._CriticalRate = companion._CritRate;
      classData._AsciiFile = companion._AsciiFile;
      
      // 직업별 플레이어 생성 (인벤토리 비활성화)
      std::shared_ptr<Player> newCompanion;
      
      if (companion._JobType == "warrior") {
        newCompanion = std::make_shared<Warrior>(classData, companion._Name, false);
      } else if (companion._JobType == "mage") {
        newCompanion = std::make_shared<Mage>(classData, companion._Name, false);
      } else if (companion._JobType == "archer") {
        newCompanion = std::make_shared<Archer>(classData, companion._Name, false);
      } else if (companion._JobType == "priest") {
        newCompanion = std::make_shared<Priest>(classData, companion._Name, false);
      }
      
      if (newCompanion) {
        // 레벨 설정 (층수와 동일)
        for (int i = 1; i < _CompanionLevel; ++i) {
          newCompanion->ProcessLevelUp();
        }
        
        party.push_back(newCompanion);
        
        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) {
          logPanel->ClearRenderers();
          auto successText = std::make_unique<TextRenderer>();
          successText->AddLine("");
          successText->AddLineWithColor(
              "  [ 시스템 로그 ]",
              MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
          successText->AddLine("");
          successText->AddLineWithColor(
              "  " + companion._Name + "이(가) 파티에 합류했습니다!",
              MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK));
          successText->AddLine("  (Enter를 눌러 계속)");
          logPanel->AddRenderer(1, 0, 74, 10, std::move(successText));
          logPanel->Redraw();
          _Drawer->Render();
          
          Sleep(1500);
        }
      }
      
      _IsActive = false;
      Exit();
      SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
    } else {
      // 거부
      _RecruitAccepted = false;
      _IsActive = false;
      Exit();
      SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
    }
  }
}

void CompanionRecruitScene::UpdateSelectionUI() {
  Panel* logPanel = _Drawer->GetPanel("SystemLog");
  if (!logPanel || !_CurrentCompanion.has_value()) return;

  logPanel->ClearRenderers();

  auto logText = std::make_unique<TextRenderer>();
  logText->AddLine("");
  logText->AddLineWithColor(
      "  [ 시스템 로그 ]",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  logText->AddLine("");
  
  if (_SelectedOption == 0) {
    logText->AddLineWithColor(
        "  > 영입하기",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    logText->AddLine("    거부하기");
  } else {
    logText->AddLine("    영입하기");
    logText->AddLineWithColor(
        "  > 거부하기",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
  }

  logPanel->AddRenderer(1, 0, 74, 10, std::move(logText));
  logPanel->Redraw();
  _Drawer->Render();
}