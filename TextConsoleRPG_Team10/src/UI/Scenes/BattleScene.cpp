#include "../../../include/UI/Scenes/BattleScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/BattleManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/StageManager.h"  // 추가
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"
#include "../../../include/Unit/IMonster.h"
#include "../../../include/Item/Inventory.h"
#include "../../../include/Item/IItem.h"
#include <Windows.h>

BattleScene::BattleScene()
    : UIScene("Battle")
    , _CurrentTurn(1)
    , _PlayerTurn(true)
    , _SelectedAction(0)
    , _SelectedTarget(0)
    , _BattleEnd(false)
    , _SelectedItemSlot(0)
    , _IsSelectingItem(false)
    , _IsPlayingAnimation(false)
    , _AnimationTimer(0.0f)
    , _CurrentAnimationName("")
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

    // 입력 버퍼 클리어
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hInput);

    // 초기화
    _SystemLogs.clear();
    _SelectedItemSlot = 0;
    _IsSelectingItem = false;
    _BattleEnd = false;

    BattleManager* battleMgr = BattleManager::GetInstance();
    GameManager* gameMgr = GameManager::GetInstance();

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 스테이지 정보 & 몬스터 HP 패널 (상단) =====
    Panel* infoPanel = _Drawer->CreatePanel("BattleInfo", 0, 0, 150, 5);
    infoPanel->SetBorder(true, ETextColor::WHITE);
    UpdateBattleInfoPanel();

    // ===== 캐릭터 아스키 아트 패널 (왼쪽, 4명 2x2 배치) =====
    int charArtStartX = 0;
    int charArtStartY = 5;
    int charArtWidth = 24;
    int charArtHeight = 12;

    auto party = gameMgr->GetParty();

    for (int i = 0; i < 4; ++i) {
        int row = i / 2;
        int col = i % 2;
        int xPos = charArtStartX + col * charArtWidth;
        int yPos = charArtStartY + row * charArtHeight;

        std::string panelName = "CharArt" + std::to_string(i);

        Panel* charArtPanel =
            _Drawer->CreatePanel(panelName, xPos, yPos, charArtWidth, charArtHeight);
        charArtPanel->SetBorder(true, ETextColor::WHITE);

        // TODO: 캐릭터별 아스키 아트 로드
        auto charArtText = std::make_unique<TextRenderer>();
        charArtText->AddLine("");
        charArtText->AddLine("");

        if (i < party.size() && party[i])
        {
            charArtText->AddLine("     [" + party[i]->GetName() + "]");
        }
        else
        {
            charArtText->AddLine("     [빈 슬롯]");
        }

        charArtText->SetTextColor(
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        charArtPanel->SetContentRenderer(std::move(charArtText));
        charArtPanel->Redraw();
    }

    // ===== 아군 파티 패널 (중단, 4명 가로 배치) =====
    int partyStartX = 2;
    int partyStartY = 29;
    int partyHeight = 6;
    int partyWidth = 36;

    for (int i = 0; i <= 3; ++i)
    {
        int xPos = partyStartX + i * (partyWidth + 1);
        std::string panelName = "Party" + std::to_string(i);

        Panel* partyPanel =
            _Drawer->CreatePanel(panelName, xPos, partyStartY, partyWidth, partyHeight);
        partyPanel->SetBorder(true, ETextColor::WHITE);
    }

    UpdatePartyPanels();

    // ===== 애니메이션 영역 (중앙) =====
    Panel* animPanel = _Drawer->CreatePanel("Animation", 48, 5, 62, 24);
    animPanel->SetBorder(true, ETextColor::WHITE);

    auto animArt = std::make_unique<AsciiArtRenderer>();
    animArt->SetAlignment(ArtAlignment::CENTER);
    animArt->SetColor(ETextColor::WHITE);
    animPanel->SetContentRenderer(std::move(animArt));
    animPanel->Redraw();

    // ===== 몬스터 이미지 패널 (오른쪽) =====
    Panel* enemyPanel = _Drawer->CreatePanel("Enemy", 110, 8, 40, 17);
    enemyPanel->SetBorder(true, ETextColor::WHITE);
    UpdateMonsterInfoPanel();

    // ===== 시스템 로그 패널 (하단 좌측-중앙), 내부 우측에 커맨드 통합 =====
    Panel* logPanel = _Drawer->CreatePanel("SystemLog", 0, 35, 113, 10);
    logPanel->SetBorder(true, ETextColor::WHITE);

    std::vector<std::string> initialLogs = {
      "[전투] 전투가 시작되었습니다!",
      "",
      "[안내] Space 키를 눌러 턴을 진행하세요."
    };
    UpdateSystemLog(logPanel, initialLogs);

    // ===== 인벤토리 패널 (하단 우측) =====
    Panel* inventoryPanel = _Drawer->CreatePanel("Inventory", 113, 35, 37, 10);
    inventoryPanel->SetBorder(true, ETextColor::WHITE);
    UpdateInventoryPanel(inventoryPanel);

    _Drawer->Render();
}

void BattleScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _SystemLogs.clear();
    _IsActive = false;
}

void BattleScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();

        // 애니메이션 타이머 업데이트
        if (_IsPlayingAnimation)
        {
            _AnimationTimer -= 0.016f;  // 약 60 FPS 기준
            if (_AnimationTimer <= 0.0f)
            {
                StopAnimation();
            }
        }

        HandleInput();
    }
}

void BattleScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

// ===== UI 업데이트 함수 (StageSelectScene 패턴) =====

void BattleScene::UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages)
{
    if (!systemPanel) return;

    systemPanel->ClearRenderers();
    systemPanel->SetDirty();

    // 좌측 영역: 시스템 로그 (1 ~ 74)
    auto logText = std::make_unique<TextRenderer>();
    logText->AddLineWithColor(
        "  [ 시스템 로그 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    int maxLines = 6;  // 로그 표시 라인 수
    int messageSize = static_cast<int>(messages.size());
    int displayCount = (messageSize < maxLines) ? messageSize : maxLines;
    int startIdx = (messageSize > maxLines) ? (messageSize - maxLines) : 0;

    for (int i = startIdx; i < messageSize; ++i)
    {
        if (messages[i].empty())
        {
            logText->AddLine("");
            continue;
        }

        WORD color;
        if (messages[i].find("[전투]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);
        else if (messages[i].find("[경고]") != std::string::npos || messages[i].find("[오류]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK);
        else if (messages[i].find("[승리]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
        else if (messages[i].find("[안내]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);
        else
            color = MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK);

        logText->AddLineWithColor(messages[i], color);
    }

    systemPanel->AddRenderer(1, 0, 74, 10, std::move(logText));

    // 우측 영역: 커맨드 (75 ~ 111)
    auto commandText = std::make_unique<TextRenderer>();
    commandText->AddLine("");
    commandText->AddLineWithColor(
        "  [ 커맨드 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    commandText->AddLine("");
    commandText->AddLine("  [Space] 턴 진행");
    commandText->AddLine("  [1~5] 아이템 예약");
    commandText->AddLine("  [C] 예약 취소");
    commandText->AddLine("  [ESC] 도망");
    systemPanel->AddRenderer(75, 0, 37, 10, std::move(commandText));

    systemPanel->Redraw();
}

void BattleScene::UpdateInventoryPanel(Panel* inventoryPanel)
{
    if (!inventoryPanel) return;

    auto inventoryText = std::make_unique<TextRenderer>();

    Player* player = GameManager::GetInstance()->GetMainPlayer().get();

    if (!player)
    {
        inventoryText->AddLineWithColor("[ 인벤토리 ]",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        inventoryText->AddLineWithColor("플레이어 정보 없음",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 35, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory) || !inventory)
    {
        inventoryText->AddLineWithColor("[ 인벤토리 ]",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        inventoryText->AddLineWithColor("인벤토리 비활성화",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 35, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    inventoryText->AddLineWithColor("[ 인벤토리 ]",
        MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));

    BattleManager* battleMgr = BattleManager::GetInstance();
    const auto& reservations = battleMgr->GetActiveReservations();

    const int maxSlots = 5;
    for (int i = 0; i < maxSlots; ++i)
    {
        IItem* item = inventory->GetItemAtSlot(i);

        // 예약 여부 체크 + 예약한 캐릭터 찾기
        bool isReserved = false;
        Player* reservedBy = nullptr;

        for (const auto& res : reservations)
        {
            if (res.IsActive && res.SlotIndex == i)
            {
                isReserved = true;
                reservedBy = res.User;
                break;
            }
        }

        if (item)
        {
            int amount = inventory->GetSlotAmount(i);
            std::string prefix = _IsSelectingItem && _SelectedItemSlot == i ? ">" : " ";

            // 예약 정보 추가 (캐릭터 이름 포함)
            std::string suffix = "";
            WORD itemColor = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

            if (isReserved && reservedBy)
            {
                int partyIdx = GetPartyIndex(reservedBy);
                itemColor = GetCharacterColor(partyIdx);
                suffix = " [" + reservedBy->GetName() + "]";
            }

            std::string itemLine = prefix + std::to_string(i + 1) + ". " +
                item->GetName() + " x" + std::to_string(amount) + suffix;

            inventoryText->AddLineWithColor(itemLine, itemColor);
        }
        else
        {
            std::string prefix = _IsSelectingItem && _SelectedItemSlot == i ? ">" : " ";
            std::string emptyLine = prefix + std::to_string(i + 1) + ". [빈 슬롯]";
            inventoryText->AddLineWithColor(emptyLine,
                MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        }
    }

    inventoryPanel->ClearRenderers();
    inventoryPanel->AddRenderer(0, 0, 35, 9, std::move(inventoryText));
    inventoryPanel->Redraw();
}

void BattleScene::UpdatePartyPanels()
{
    auto party = GameManager::GetInstance()->GetParty();
    BattleManager* battleMgr = BattleManager::GetInstance();
    const auto& reservations = battleMgr->GetActiveReservations();

    for (int i = 0; i < 4; ++i)
    {
        std::string panelName = "Party" + std::to_string(i);
        Panel* partyPanel = _Drawer->GetPanel(panelName);
        if (!partyPanel) continue;

        auto partyText = std::make_unique<TextRenderer>();
        partyText->AddLine("");

        if (i < party.size() && party[i])
        {
            std::string name = party[i]->GetName();
            std::string className = "전사";  // TODO: 직업 추가
            int hp = party[i]->GetCurrentHP();
            int maxHp = party[i]->GetMaxHP();
            int mp = party[i]->GetCurrentMP();
            int maxMp = party[i]->GetMaxMP();

            // 예약 여부 체크
            bool hasReservation = false;
            for (const auto& res : reservations)
            {
                if (res.IsActive && res.User == party[i].get())
                {
                    hasReservation = true;
                    break;
                }
            }

            // 이름 색상 (예약 있으면 캐릭터 색상, 없으면 기본 색상)
            WORD nameColor = hasReservation ?
                GetCharacterColor(i) :
                MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);

            partyText->AddLineWithColor(" " + name + " (" + className + ")",
                nameColor);

            std::string hpLine = " HP:" + std::to_string(hp) + "/" + std::to_string(maxHp);
            WORD hpColor = (hp < maxHp * 0.3f) ?
                MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK) :
                MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
            partyText->AddLineWithColor(hpLine, hpColor);

            std::string mpLine = " MP:" + std::to_string(mp) + "/" + std::to_string(maxMp);
            partyText->AddLineWithColor(mpLine,
                MakeColorAttribute(ETextColor::LIGHT_BLUE, EBackgroundColor::BLACK));
        }
        else
        {
            partyText->AddLine(" [빈 슬롯]");
            partyText->AddLine("");
        }

        partyPanel->ClearRenderers();
        partyPanel->AddRenderer(0, 0, 34, 4, std::move(partyText));
        partyPanel->Redraw();
    }
}

void BattleScene::UpdateMonsterInfoPanel()
{
    Panel* enemyPanel = _Drawer->GetPanel("Enemy");
    if (!enemyPanel) return;

    BattleManager* battleMgr = BattleManager::GetInstance();
    IMonster* monster = battleMgr->GetCurrentMonster();

    if (!monster)
    {
        auto errorText = std::make_unique<TextRenderer>();
        errorText->AddLine("");
        errorText->AddLine("");
        errorText->AddLine("    [몬스터 없음]");
        errorText->SetTextColor(
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        enemyPanel->SetContentRenderer(std::move(errorText));
        enemyPanel->Redraw();
        return;
    }

    // TODO: 몬스터별 아스키 아트 로드
    auto monsterArt = std::make_unique<AsciiArtRenderer>();

    std::string monstersPath = DataManager::GetInstance()->GetResourcePath("Monsters");
    std::string artFile = monster->GetName() + ".txt";  // 몬스터 이름으로 파일 검색

    if (monsterArt->LoadFromFile(monstersPath, artFile))
    {
        monsterArt->SetAlignment(ArtAlignment::CENTER);
        monsterArt->SetColor(ETextColor::LIGHT_RED);
        enemyPanel->SetContentRenderer(std::move(monsterArt));
    }
    else
    {
        auto fallbackText = std::make_unique<TextRenderer>();
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLineWithColor("  [" + monster->GetName() + "]",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        fallbackText->AddLine("");
        fallbackText->AddLineWithColor("  HP: " + std::to_string(monster->GetCurrentHP()) +
            "/" + std::to_string(monster->GetMaxHP()),
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        enemyPanel->SetContentRenderer(std::move(fallbackText));
    }

    enemyPanel->Redraw();
}

void BattleScene::UpdateBattleInfoPanel()
{
    Panel* infoPanel = _Drawer->GetPanel("BattleInfo");
    if (!infoPanel) return;

    BattleManager* battleMgr = BattleManager::GetInstance();
    IMonster* monster = battleMgr->GetCurrentMonster();

    auto infoText = std::make_unique<TextRenderer>();
    infoText->AddLine("");

    std::string battleTypeStr = "";
    switch (battleMgr->GetBattleType())
    {
    case EBattleType::Normal: battleTypeStr = "[일반 전투]"; break;
    case EBattleType::Elite:  battleTypeStr = "[정예 전투]"; break;
    case EBattleType::Boss:   battleTypeStr = "[보스 전투]"; break;
    default: battleTypeStr = "[전투]"; break;
    }

    if (monster)
    {
        std::string info = "      " + battleTypeStr + " - " +
            monster->GetName() + " (HP: " +
            std::to_string(monster->GetCurrentHP()) + "/" +
            std::to_string(monster->GetMaxHP()) + ")";
        infoText->AddLineWithColor(info,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    }
    else
    {
        infoText->AddLineWithColor("               전투 대기 중...",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
    }

    infoPanel->ClearRenderers();
    infoPanel->AddRenderer(0, 0, 148, 3, std::move(infoText));
    infoPanel->Redraw();
}

// ===== 입력 처리 =====

void BattleScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;

    int keyCode = input->GetKeyCode();
    if (keyCode == 0 || keyCode < 0) return;

    BattleManager* battleMgr = BattleManager::GetInstance();
    Player* mainPlayer = GameManager::GetInstance()->GetMainPlayer().get();

    if (!mainPlayer) return;

    // ESC: 도망 (전투 포기)
    if (keyCode == VK_ESCAPE)
    {
        _SystemLogs.push_back("[경고] 전투에서 도망쳤습니다!");
        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

        Sleep(1000);

        battleMgr->EndBattle();
        _IsActive = false;
        Exit();
        SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
        return;
    }

    // Space: 턴 진행
    if (keyCode == VK_SPACE)
    {
        if (_BattleEnd)
        {
            // 전투 종료 후 Space 누르면 다음 씬으로
            EndBattle(battleMgr->GetBattleResult().Victory);
            return;
        }

        ProcessBattleTurn();
        return;
    }

    // 1~5: 아이템 예약/예약 취소
    if (keyCode >= '1' && keyCode <= '5')
    {
        int slotIndex = keyCode - '1';  // 0~4

        Inventory* inventory = nullptr;
        if (!mainPlayer->TryGetInventory(inventory) || !inventory)
        {
            _SystemLogs.push_back("[오류] 인벤토리를 사용할 수 없습니다.");
            Panel* logPanel = _Drawer->GetPanel("SystemLog");
            if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);
            return;
        }

        IItem* item = inventory->GetItemAtSlot(slotIndex);
        if (!item)
        {
            _SystemLogs.push_back("[경고] 해당 슬롯에 아이템이 없습니다.");
            Panel* logPanel = _Drawer->GetPanel("SystemLog");
            if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);
            return;
        }

        // 이미 예약되어 있으면 취소
        if (item->IsReserved())
        {
            if (battleMgr->CancelItemReservation(mainPlayer, slotIndex))
            {
                _SystemLogs.push_back("[안내] " + item->GetName() + " 예약이 취소되었습니다.");
            }
            else
            {
                _SystemLogs.push_back("[오류] 예약 취소 실패.");
            }
        }
        else
        {
            // 예약 등록
            if (battleMgr->ReserveItemUse(mainPlayer, slotIndex))
            {
                _SystemLogs.push_back("[안내] " + item->GetName() + " 예약 완료! (" +
                    item->GetUseConditionDescription() + ")");
            }
            else
            {
                _SystemLogs.push_back("[오류] 예약 실패.");
            }
        }

        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

        Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
        if (inventoryPanel) UpdateInventoryPanel(inventoryPanel);

        _Drawer->Render();
        return;
    }

    // C: 모든 예약 취소
    if (keyCode == 'C' || keyCode == 'c')
    {
        Inventory* inventory = nullptr;
        if (!mainPlayer->TryGetInventory(inventory)) return;

        const auto& reservations = battleMgr->GetActiveReservations();
        int cancelCount = 0;

        for (const auto& res : reservations)
        {
            if (res.IsActive && res.User == mainPlayer)
            {
                if (battleMgr->CancelItemReservation(mainPlayer, res.SlotIndex))
                {
                    cancelCount++;
                }
            }
        }

        if (cancelCount > 0)
        {
            _SystemLogs.push_back("[안내] " + std::to_string(cancelCount) + "개의 예약이 취소되었습니다.");
        }
        else
        {
            _SystemLogs.push_back("[안내] 취소할 예약이 없습니다.");
        }

        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

        Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
        if (inventoryPanel) UpdateInventoryPanel(inventoryPanel);

        _Drawer->Render();
        return;
    }
}

// ===== 전투 로직 연동 =====

void BattleScene::ProcessBattleTurn()
{
    BattleManager* battleMgr = BattleManager::GetInstance();

    if (!battleMgr->IsBattleActive())
    {
        _SystemLogs.push_back("[오류] 전투가 활성화되지 않았습니다.");
        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);
        return;
    }

    _SystemLogs.push_back("");
    _SystemLogs.push_back("[전투] === 라운드 " + std::to_string(battleMgr->GetCurrentRound() + 1) + " 시작 ===");

    // TODO: 애니메이션 재생
    // PlayAnimation("BattleStart", 1.0f);

    // BattleManager 턴 처리
    bool continuesBattle = battleMgr->ProcessBattleTurn();

    // UI 업데이트
    UpdatePartyPanels();
    UpdateMonsterInfoPanel();
    UpdateBattleInfoPanel();

    // 전투 종료 체크
    if (!continuesBattle)
    {
        const BattleResult& result = battleMgr->GetBattleResult();

        if (result.Victory)
        {
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[승리] 전투에서 승리했습니다!");
            _SystemLogs.push_back("[보상] 경험치: " + std::to_string(result.ExpGained) +
                ", 골드: " + std::to_string(result.GoldGained) + "G");
            if (!result.ItemName.empty())
            {
                _SystemLogs.push_back("[보상] 아이템 획득: " + result.ItemName);
            }
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[안내] Space 키를 눌러 계속하세요.");
        }
        else
        {
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[패배] 전투에서 패배했습니다...");
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[안내] Space 키를 눌러 계속하세요.");
        }

        _BattleEnd = true;
    }

    Panel* logPanel = _Drawer->GetPanel("SystemLog");
    if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

    Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
    if (inventoryPanel) UpdateInventoryPanel(inventoryPanel);

    _Drawer->Render();
}

void BattleScene::EndBattle(bool victory)
{
    BattleManager* battleMgr = BattleManager::GetInstance();
    StageManager* stageMgr = StageManager::GetInstance();

    // ===== 1. 승리 시 StageManager에 완료 표시 =====
    if (victory)
    {
        EBattleType battleType = battleMgr->GetBattleType();

        // 노드 타입 결정
        ENodeType nodeType = ENodeType::Battle;

        // 현재 노드 완료 처리
        stageMgr->CompleteNode(nodeType);

        // 로그 추가
        _SystemLogs.push_back("");
        _SystemLogs.push_back("[성공] 스테이지 클리어!");
    }

    // ===== 2. 보상 정보 최종 표시 (패널 업데이트) =====
    Panel* logPanel = _Drawer->GetPanel("SystemLog");
    if (logPanel)
    {
        UpdateSystemLog(logPanel, _SystemLogs);
    }

    _Drawer->Render();

    // ===== 3. 사용자 입력 대기 (보상 확인용) =====
    Sleep(2000);  // 2초 대기 (보상 읽을 시간)

    // ===== 4. BattleManager 정리 =====
    battleMgr->EndBattle();

    _IsActive = false;
    Exit();

    // ===== 5. 씬 전환 =====
    if (victory)
    {
        // 승리 시 StageSelect로 복귀
        SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
    }
    else
    {
        // 패배 시 메인 메뉴로 이동
        SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
    }
}

// ===== 애니메이션 =====

void BattleScene::PlayAnimation(const std::string& animationName, float duration)
{
    Panel* animPanel = _Drawer->GetPanel("Animation");
    if (!animPanel) return;

    auto animArt = std::make_unique<AsciiArtRenderer>();

    std::string animPath = DataManager::GetInstance()->GetResourcePath("Animations");
    std::string jsonFile = animationName + ".json";

    if (animArt->LoadAnimationFromJson(animPath, jsonFile))
    {
        animArt->SetAlignment(ArtAlignment::CENTER);
        animArt->SetColor(ETextColor::LIGHT_YELLOW);
        animArt->StartAnimation();

        animPanel->SetContentRenderer(std::move(animArt));
        animPanel->Redraw();

        _IsPlayingAnimation = true;
        _AnimationTimer = duration;
        _CurrentAnimationName = animationName;
    }
    else
    {
        // 애니메이션 로드 실패 - 기본 텍스트 표시
        auto fallbackText = std::make_unique<TextRenderer>();
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLine("  [" + animationName + "]");
        fallbackText->SetTextColor(
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        animPanel->SetContentRenderer(std::move(fallbackText));
        animPanel->Redraw();
    }
}

void BattleScene::StopAnimation()
{
    Panel* animPanel = _Drawer->GetPanel("Animation");
    if (!animPanel) return;

    auto emptyText = std::make_unique<TextRenderer>();
    emptyText->AddLine("");
    emptyText->AddLine("");
    emptyText->AddLine("  [대기 중]");
    emptyText->SetTextColor(
        MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
    animPanel->SetContentRenderer(std::move(emptyText));
    animPanel->Redraw();

    _IsPlayingAnimation = false;
    _AnimationTimer = 0.0f;
    _CurrentAnimationName = "";
}

// ===== 헬퍼 함수 =====

WORD BattleScene::GetCharacterColor(int partyIndex) const
{
    switch (partyIndex)
    {
    case 0: return MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);    // 메인 - 하늘색
    case 1: return MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);   // 동료1 - 연두색
    case 2: return MakeColorAttribute(ETextColor::LIGHT_MAGENTA, EBackgroundColor::BLACK); // 동료2 - 분홍색
    case 3: return MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK);  // 동료3 - 노란색
    default: return MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);
    }
}

int BattleScene::GetPartyIndex(Player* player) const
{
    if (!player) return -1;

    auto party = GameManager::GetInstance()->GetParty();
    for (size_t i = 0; i < party.size(); ++i)
    {
        if (party[i].get() == player)
        {
            return static_cast<int>(i);
        }
    }

    return -1;
}
