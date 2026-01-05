#include "../../../include/UI/Scenes/BattleScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"

BattleScene::BattleScene()
    : UIScene("Battle")
    , _CurrentTurn(1)
    , _PlayerTurn(true)
    , _SelectedAction(0)
    , _SelectedTarget(0)
    , _BattleEnd(false)
{
}

BattleScene::~BattleScene()
{
}

void BattleScene::Enter() {
  _Drawer->ClearScreen();
  _Drawer->RemoveAllPanels();
  _Drawer->Activate();
  _IsActive = true;

  // =============================================================================
  // 패널 레이아웃 (150x45 화면 기준)
  // =============================================================================

  // ===== 스테이지 정보 & 몬스터 HP 패널 (상단) =====
  Panel* infoPanel = _Drawer->CreatePanel("BattleInfo", 0, 0, 150, 5);
  infoPanel->SetBorder(true, ETextColor::WHITE);

  auto infoText = std::make_unique<TextRenderer>();
  infoText->AddLineWithColor(
      "                           [스테이지 명] - 몬스터 이름 (남은 HP/총 HP)",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  infoPanel->SetContentRenderer(std::move(infoText));
  infoPanel->Redraw();

  // ===== 캐릭터 아스키 아트 패널 (왼쪽, 4명 2x2 배치) =====
  int charArtStartX = 0;
  int charArtStartY = 5;
  int charArtWidth = 24;
  int charArtHeight = 12;  

  for (int i = 0; i < 4; ++i) {
    int row = i / 2;  // 0, 0, 1, 1
    int col = i % 2;  // 0, 1, 0, 1
    int xPos = charArtStartX + col * charArtWidth;
    int yPos = charArtStartY + row * charArtHeight;

    std::string panelName = "CharArt" + std::to_string(i);

    Panel* charArtPanel =
        _Drawer->CreatePanel(panelName, xPos, yPos, charArtWidth, charArtHeight);
    charArtPanel->SetBorder(true, ETextColor::WHITE);

    auto charArtText = std::make_unique<TextRenderer>();
    charArtText->AddLine("");
    charArtText->AddLine("");
    charArtText->AddLine("     [캐릭터");
    charArtText->AddLine("      아트 " + std::to_string(i) + "]");
    charArtText->SetTextColor(
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    charArtPanel->SetContentRenderer(std::move(charArtText));
    charArtPanel->Redraw();
  }

  // ===== 아군 파티 패널 (상단, 4명 가로 배치) =====
  // 왼쪽부터 메인 플레이어, 용병 1, 용병 2, 용병 3 순서
  auto party = GameManager::GetInstance()->GetParty();

  int partyStartX = 2;
  int partyStartY = 29;
  int partyHeight = 7;
  int partyWidth = 36;

  for (int i = 0; i <= 3; ++i)  
  {
    int xPos = partyStartX + i * (partyWidth + 1);
    std::string panelName = "Party" + std::to_string(i);

    Panel* partyPanel =
        _Drawer->CreatePanel(panelName, xPos, partyStartY, partyWidth, partyHeight);
    partyPanel->SetBorder(true, ETextColor::WHITE);

    auto partyText = std::make_unique<TextRenderer>();
    partyText->AddLine("");

    if (i < party.size() && party[i]) {
      std::string name = party[i]->GetName();
      std::string className = "전사";  // TODO: 직업 추가
      int hp = party[i]->GetCurrentHP();
      int maxHp = party[i]->GetMaxHP();
      int atk = party[i]->GetAtk();
      int def = party[i]->GetDef();

      partyText->AddLine(" 이름:" + name + " | 직업:" + className);
      partyText->AddLine(" HP:" + std::to_string(hp) + "/" + std::to_string(maxHp) +
                         " | ATK:" + std::to_string(atk) + " DEF:" + std::to_string(def));
      partyText->AddLine("");
    } else {
      partyText->AddLine(" 빈 슬롯");
      partyText->AddLine("");
    }

    partyPanel->SetContentRenderer(std::move(partyText));
    partyPanel->Redraw();
  }

  // ===== 애니메이션 영역 (중앙) =====
  Panel* animPanel = _Drawer->CreatePanel("Animation", 48, 5, 62, 24);
  animPanel->SetBorder(true, ETextColor::WHITE);
  auto animText = std::make_unique<TextRenderer>();
  animText->AddLine("");
  animText->AddLine("");
  animText->AddLine("");
  animText->AddLine("  [애니메이션 아스키아트 재생 영역]");
  animText->AddLine("");
  animText->SetTextColor(
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  animPanel->SetContentRenderer(std::move(animText));
  animPanel->Redraw();

  // ===== 몬스터 이미지 패널 (오른쪽) =====
  Panel* enemyPanel = _Drawer->CreatePanel("Enemy", 110, 8, 40, 17);
  enemyPanel->SetBorder(true, ETextColor::WHITE);

  // TODO: Monsters.csv에서 몬스터 정보 읽어서 표시
  // TODO: Resources/Monsters/{FileName}.txt에서 아스키 아트 로드
  auto enemyText = std::make_unique<TextRenderer>();
  enemyText->AddLine("");
  enemyText->AddLine("");
  enemyText->AddLine("");
  enemyText->AddLine("           [몬스터 이미지]");
  enemyText->AddLine("");
  enemyText->AddLine("      (Resources/Monsters/*.txt)");
  enemyText->SetTextColor(
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  enemyPanel->SetContentRenderer(std::move(enemyText));
  enemyPanel->Redraw();

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
  logText->AddLine("  전투가 시작되었습니다!");
  logPanel->AddRenderer(1, 0, 74, 10, std::move(logText));

  // 우측 영역: 커맨드 (75 ~ 111)
  auto commandText = std::make_unique<TextRenderer>();
  commandText->AddLine("");
  commandText->AddLineWithColor(
      "  [ 커맨드 ]",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  commandText->AddLine("");
  commandText->AddLine("  [1] 공격");
  commandText->AddLine("  [2] 스킬");
  commandText->AddLine("  [3] 아이템");
  commandText->AddLine("  [ESC] 도망");
  logPanel->AddRenderer(75, 0, 37, 10, std::move(commandText));

  logPanel->Redraw();

  // ===== 인벤토리 패널 (하단 우측) =====
  Panel* inventoryCommandPanel = _Drawer->CreatePanel("Command", 113, 36, 37, 9);
  inventoryCommandPanel->SetBorder(true, ETextColor::WHITE);
  auto inventoryCommandText = std::make_unique<TextRenderer>();
  inventoryCommandText->AddLine("");
  inventoryCommandText->AddLineWithColor(
      "  인벤토리",
      MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
  inventoryCommandText->AddLine("");
  inventoryCommandText->AddLine("  아이템 사용");
  inventoryCommandText->AddLine("  정보 확인");
  inventoryCommandPanel->SetContentRenderer(std::move(inventoryCommandText));
  inventoryCommandPanel->Redraw();

  _Drawer->Render();
}

void BattleScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void BattleScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        // TODO: 전투 로직 업데이트
          // BattleManager를 사용하여 전투 진행 상황을 처리합니다.
        HandleInput();
    }
}

void BattleScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void BattleScene::HandleInput()
{
    // TODO: 입력 처리 구현
    // VK_UP/VK_DOWN으로 행동 선택
    // VK_RETURN으로 행동 확정
    // 공격, 스킬, 아이템, 도망 처리
}
