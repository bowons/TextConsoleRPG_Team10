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
#include "../../../include/Manager/StageManager.h"
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"
#include "../../../include/Unit/Warrior.h"
#include "../../../include/Unit/Mage.h"
#include "../../../include/Unit/Archer.h"
#include "../../../include/Unit/Priest.h"
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
    , _SelectedPartyIndex(0)
    , _IsCancelMode(false)
    , _IsWaitingForAnimation(false)
    , _AnimationWaitTimer(0.0f)
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
    _SelectedPartyIndex = 0;
    _IsCancelMode = false;
    _BattleEnd = false;
    _InputState = EBattleInputState::Playing;
    BattleManager* battleMgr = BattleManager::GetInstance();
    GameManager* gameMgr = GameManager::GetInstance();

    // ===== 애니메이션 콜백 등록 =====
    battleMgr->SetAnimationCallback(this);

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

 // ===== 스테이지 정보 & 몬스터 HP 패널 (상단) =====
    Panel* infoPanel = _Drawer->CreatePanel("BattleInfo", 0, 0, 113, 5);
    infoPanel->SetBorder(true, ETextColor::WHITE);
    UpdateBattleInfoPanel();

    // ===== 커맨드 패널 (상단 우측 모서리) =====
    Panel* commandPanel = _Drawer->CreatePanel("Command", 113, 0, 37, 5);
    commandPanel->SetBorder(true, ETextColor::WHITE);
    UpdateCommandPanel();

    // ===== 캐릭터 아스키 아트 패널 (왼쪽, 4명 2x2 배치) =====
    int charArtStartX = 0;
    int charArtStartY = 5;
    int charArtWidth = 24;
    int charArtHeight = 12;

    auto party = gameMgr->GetParty();

    for (int i = 0; i < 4; ++i)
    {
        int row = i / 2;
        int col = i % 2;
        int xPos = charArtStartX + col * charArtWidth;
        int yPos = charArtStartY + row * charArtHeight;

        std::string panelName = "CharArt" + std::to_string(i);

        Panel* charArtPanel =
            _Drawer->CreatePanel(panelName, xPos, yPos, charArtWidth, charArtHeight);
        charArtPanel->SetBorder(true, ETextColor::WHITE);

        // 캐릭터별 아스키 아트 로드
        if (i < party.size() && party[i])
        {
            Player* player = party[i].get();
            
            // CSV에서 정의된 파일명 사용 (Class.csv의 ascii_file 컬럼)
            std::string artFileName;
            
            if (dynamic_cast<Warrior*>(player)) {
                artFileName = "P_Warrior.txt";  // CSV: Characters/P_Warrior.txt
            }
            else if (dynamic_cast<Mage*>(player)) {
                artFileName = "P_Mage.txt";     // CSV: Characters/P_Mage.txt
            }
            else if (dynamic_cast<Archer*>(player)) {
                artFileName = "P_Archer.txt";   // CSV: Characters/P_Archer.txt
            }
            else if (dynamic_cast<Priest*>(player)) {
                artFileName = "P_Priest.txt";   // CSV: Characters/P_Priest.txt
            }
            else {
                artFileName = "P_" + player->GetName() + ".txt";
            }

            auto charArt = std::make_unique<AsciiArtRenderer>();
            std::string charactersPath = DataManager::GetInstance()->GetResourcePath("Characters");
            
            if (charArt->LoadFromFile(charactersPath, artFileName))
            {
                charArt->SetAlignment(ArtAlignment::CENTER);
                charArt->SetColor(ETextColor::WHITE);
                charArtPanel->SetContentRenderer(std::move(charArt));
            }
            else
            {
                // 로드 실패 시 기본 텍스트
                auto fallbackText = std::make_unique<TextRenderer>();
                fallbackText->AddLine("");
                fallbackText->AddLine("");
                fallbackText->AddLine("  [" + player->GetName() + "]");
                fallbackText->AddLine("");
                fallbackText->AddLineWithColor("  (이미지 없음)",
                    MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
                fallbackText->SetTextColor(
                    MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
                charArtPanel->SetContentRenderer(std::move(fallbackText));
            }
        }
        else
        {
            // 빈 슬롯
            auto emptyText = std::make_unique<TextRenderer>();
            emptyText->AddLine("");
            emptyText->AddLine("");
            emptyText->AddLine("     [빈 슬롯]");
            emptyText->SetTextColor(
                MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
            charArtPanel->SetContentRenderer(std::move(emptyText));
        }

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
    // ===== BattleManager → BattleScene 로그 Flush 연결 =====
    BattleManager::GetInstance()->SetFlushCallback(
        [this](EBattleFlushType type)
        {
            this->CollectBattleLogs();
        }
    );
}

void BattleScene::Exit()
{
    // 🔥 중요: Flush 콜백 해제
    BattleManager::GetInstance()->SetFlushCallback(nullptr);

    _Drawer->RemoveAllPanels();
    _SystemLogs.clear();
    _IsActive = false;
}

void BattleScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();

        // 애니메이션 대기 타이머 업데이트
        if (_IsWaitingForAnimation)
        {
            _AnimationWaitTimer -= 0.016f;// ~60 FPS
            if (_AnimationWaitTimer <= 0.0f)
            {
                _IsWaitingForAnimation = false;
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

    // ===== 전체 영역: 시스템 로그만 표시 =====
    auto logText = std::make_unique<TextRenderer>();
    logText->AddLineWithColor(
        "  [ 시스템 로그 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    int maxLines = 7;  // 로그 표시 라인 수
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
            color = MakeColorAttribute(ETextColor::LIGHT_MAGENTA, EBackgroundColor::BLACK);

        logText->AddLineWithColor(messages[i], color);
    }

    systemPanel->AddRenderer(1, 0, 111, 10, std::move(logText));
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

        // 예약 여부 체크 + 예약 대상 찾기
        bool isReserved = false;
        Player* reservedTarget = nullptr;

        for (const auto& res : reservations)
        {
            if (res.IsActive && res.SlotIndex == i)
            {
                isReserved = true;
                reservedTarget = res.Target;
                break;
            }
        }

        if (item)
        {
            int amount = inventory->GetSlotAmount(i);
            std::string prefix = _IsSelectingItem && _SelectedItemSlot == i ? ">" : " ";

            // Reservation 상태에 따라 색상 변경
            WORD itemColor = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);
            if (isReserved && reservedTarget)
            {
                int partyIdx = GetPartyIndex(reservedTarget);
                itemColor = GetCharacterColor(partyIdx);
            }

            std::string itemLine = prefix + std::to_string(i + 1) + ". " +
                item->GetName() + " x" + std::to_string(amount);

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

void BattleScene::UpdateCommandPanel()
{
    Panel* commandPanel = _Drawer->GetPanel("Command");
    if (!commandPanel) return;

    commandPanel->ClearRenderers();
    commandPanel->SetDirty();

    auto commandText = std::make_unique<TextRenderer>();
    commandText->AddLineWithColor(
        " [ 커맨드 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    commandText->AddLine(" [Space] 턴 진행");
    commandText->AddLine(" [1~5] 아이템 선택");
    commandText->AddLine(" [←→] 대상 선택");
    commandText->AddLine(" [C] 예약 취소");
    commandText->AddLine("");

    // 현재 활성화된 예약 표시
    BattleManager* battleMgr = BattleManager::GetInstance();
    const auto& reservations = battleMgr->GetActiveReservations();

    for (const auto& res : reservations)
    {
        if (!res.IsActive) continue;

        Player* target = res.Target;
        int slotIndex = res.SlotIndex;

        int partyIdx = GetPartyIndex(target);
        WORD userColor = (partyIdx != -1) ? GetCharacterColor(partyIdx) : MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

        commandText->AddLineWithColor("  [아이템 예약] " + target->GetName() + " - 슬롯 " + std::to_string(slotIndex + 1),
            userColor);
    }

    commandPanel->ClearRenderers();
    commandPanel->AddRenderer(0, 0, 35, 5, std::move(commandText));
    commandPanel->Redraw();
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

        // ===== 보더 색상 결정 =====
        WORD borderColor = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

        if (_IsCancelMode && _SelectedPartyIndex == i)
        {
            // 취소 모드에서 선택된 파티원 - MAGENTA
            borderColor = MakeColorAttribute(ETextColor::LIGHT_MAGENTA, EBackgroundColor::BLACK);
        }
        else if (_IsSelectingItem && _SelectedPartyIndex == i)
        {
            // 아이템 선택 모드에서 선택된 파티원 - 캐릭터 색상
            borderColor = GetCharacterColor(i);
        }
        else
        {
            // 예약 확인 - 예약이 있으면 캐릭터 색상
            bool hasReservation = false;

            // ===== 범위 체크 추가 =====
            if (i < party.size() && party[i])
            {
                for (const auto& res : reservations)
                {
                    // Target이 현재 파티원과 일치하는지 확인
                    if (res.IsActive && res.Target == party[i].get())
                    {
                        hasReservation = true;
                        break;
                    }
                }
            }

            if (hasReservation)
            {
                borderColor = GetCharacterColor(i);
            }
        }

        partyPanel->SetBorder(true, borderColor);

        auto partyText = std::make_unique<TextRenderer>();

        if (i < party.size() && party[i])
        {
            std::string name = party[i]->GetName();

            // ===== 직업 판별 =====
            std::string className = "Unknown";
            if (dynamic_cast<Warrior*>(party[i].get())) {
                className = "전사";
            }
            else if (dynamic_cast<Mage*>(party[i].get())) {
                className = "마법사";
            }
            else if (dynamic_cast<Archer*>(party[i].get())) {
                className = "궁수";
            }
            else if (dynamic_cast<Priest*>(party[i].get())) {
                className = "사제";
            }

            // ===== 스탯 정보 =====
            int level = party[i]->GetLevel();
            int hp = party[i]->GetCurrentHP();
            int maxHp = party[i]->GetMaxHP();
            int mp = party[i]->GetCurrentMP();
            int maxMp = party[i]->GetMaxMP();
            int atk = party[i]->GetTotalAtk();
            int def = party[i]->GetTotalDef();
            int dex = party[i]->GetTotalDex();
            int luk = party[i]->GetTotalLuk();
            float critRate = party[i]->GetTotalCriticalRate();
            int aggro = party[i]->GetAggro();

            // ===== 이름 + 직업 + 어그로 =====
            WORD nameColor = MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);
            std::string nameLine = "Lv." + std::to_string(level) + "-" + name + "/" + className + " (AG:" + std::to_string(aggro) + ")";
            partyText->AddLineWithColor(nameLine, nameColor);

            // ===== HP (빨강/초록) =====
            std::string hpLine = " HP:" + std::to_string(hp) + "/" + std::to_string(maxHp);
            WORD hpColor = (hp < maxHp * 0.3f) ?
                MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK) :
                MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
            partyText->AddLineWithColor(hpLine, hpColor);

            // ===== MP (파랑) =====
            std::string mpLine = " MP:" + std::to_string(mp) + "/" + std::to_string(maxMp);
            partyText->AddLineWithColor(mpLine,
                MakeColorAttribute(ETextColor::LIGHT_BLUE, EBackgroundColor::BLACK));

            // ===== ATK/DEF/DEX/LUK/CRIT (회색) =====
            std::string atkDefLine = " A:" + std::to_string(atk) + " D:" + std::to_string(def);
            int critPercent = static_cast<int>(critRate * 100);
            std::string dexLukLine = " Dx:" + std::to_string(dex) +
                " Lk:" + std::to_string(luk) +
                " Cr:" + std::to_string(critPercent) + "%";

            std::string statLine = atkDefLine + dexLukLine;
            partyText->AddLineWithColor(statLine,
                MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

        }
        else
        {
            partyText->AddLine("");
            partyText->AddLine(" [빈 슬롯]");
            partyText->AddLine("");
        }

        partyPanel->ClearRenderers();
        partyPanel->AddRenderer(0, 0, 34, 6, std::move(partyText));
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

    // ===== CSV 파일명 매핑 로직 =====
    // Enemy_Normal.csv, Enemy_Elite.csv, Enemy_Boss.csv의 ascii_file 컬럼 사용
    // 예: "Monsters/Slime.txt" → "Slime.txt"
    
    std::string monstersPath = DataManager::GetInstance()->GetResourcePath("Monsters");
    
    // 몬스터 이름 기반으로 파일명 추출
    // CSV 데이터: ascii_file = "Monsters/Slime.txt"
    // 실제 필요한 파일명: "Slime.txt"
    
    std::string fileName;
    std::string monsterName = monster->GetName();
    
    // ===== 몬스터 이름 → 파일명 매핑 (Monsters.csv 기준) =====
    if (monsterName.find("슬라임") != std::string::npos || monsterName.find("망령") != std::string::npos) {
        fileName = "Slime.txt";
    }
    else if (monsterName.find("쥐") != std::string::npos) {
        fileName = "Mouse.txt";
    }
    else if (monsterName.find("고블린") != std::string::npos) {
        fileName = "Goblin.txt";
    }
    else if (monsterName.find("박쥐") != std::string::npos) {
        fileName = "Bat.txt";
    }
    else if (monsterName.find("해골") != std::string::npos || monsterName.find("스켈레톤") != std::string::npos) {
        fileName = "Skeleton.txt";
    }
    else if (monsterName.find("좀비") != std::string::npos) {
        fileName = "Zombie.txt";
    }
    else if (monsterName.find("가고일") != std::string::npos || monsterName.find("조각상") != std::string::npos) {
        fileName = "Gargoyle.txt";
    }
    else if (monsterName.find("수호자") != std::string::npos || monsterName.find("집사") != std::string::npos) {
        fileName = "Butler.txt";
    }
    else if (monsterName.find("하피") != std::string::npos) {
        fileName = "Harpy.txt";
    }
    else if (monsterName.find("바실리스크") != std::string::npos) {
        fileName = "Basilisk.txt";
    }
    else if (monsterName.find("늑대") != std::string::npos || monsterName.find("팽") != std::string::npos || monsterName.find("하운드") != std::string::npos) {
        fileName = "Wolf.txt";
    }
    else if (monsterName.find("예티") != std::string::npos || monsterName.find("골렘") != std::string::npos) {
        fileName = "Yeti.txt";
    }
    else if (monsterName.find("워커") != std::string::npos) {
        fileName = "Walker.txt";
    }
    else if (monsterName.find("데몬") != std::string::npos) {
        fileName = "Demmon.txt";
    }
    else if (monsterName.find("비홀더") != std::string::npos) {
        fileName = "Beholder.txt";
    }
    else if (monsterName.find("호러") != std::string::npos) {
        fileName = "Horror.txt";
    }
    else if (monsterName.find("카오스") != std::string::npos) {
        fileName = "Chaos.txt";
    }
    else if (monsterName.find("에테르노") != std::string::npos) {
        fileName = "Boss.txt";
    }
    else {
        // 기본 폴백
        fileName = "Slime.txt";
    }

    // 아스키 아트 로드
    auto monsterArt = std::make_unique<AsciiArtRenderer>();
    
    if (monsterArt->LoadFromFile(monstersPath, fileName))
    {
        monsterArt->SetAlignment(ArtAlignment::CENTER);
        monsterArt->SetColor(ETextColor::WHITE);
        enemyPanel->SetContentRenderer(std::move(monsterArt));
    }
    else
    {
        // 로드 실패 시 기본 텍스트
        auto fallbackText = std::make_unique<TextRenderer>();
        fallbackText->AddLine("");
        fallbackText->AddLine("");
        fallbackText->AddLineWithColor("  [ " + monster->GetName() + " ]",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        fallbackText->AddLine("");
        fallbackText->AddLineWithColor("  (이미지 로드 실패)",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        fallbackText->AddLineWithColor("  파일: " + fileName,
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        
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

    infoPanel->ClearRenderers();
    infoPanel->SetDirty();

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
        std::string info = "    " + battleTypeStr + " - " +
            monster->GetName() + " (HP: " +
            std::to_string(monster->GetCurrentHP()) + "/" +
            std::to_string(monster->GetMaxHP()) + ")";
        infoText->AddLineWithColor(info,
            MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
    }
    else
    {
        infoText->AddLineWithColor("   전투 대기 중...",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
    }

    infoPanel->AddRenderer(0, 0, 111, 5, std::move(infoText));
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
    auto party = GameManager::GetInstance()->GetParty();

    if (!mainPlayer) return;

    // ===== 방향키 처리 (아이템 선택 모드 또는 취소 모드에서) =====
    if (_IsSelectingItem || _IsCancelMode)
    {
        if (keyCode == 75)  // 왼쪽 화살표
        {
            // 이전 파티원 선택 (빈 슬롯은 건너뛰기)
            int attempts = 0;
            do {
                _SelectedPartyIndex--;
                if (_SelectedPartyIndex < 0) _SelectedPartyIndex = 3;
                attempts++;
            } while (attempts < 4 && (_SelectedPartyIndex >= party.size() || !party[_SelectedPartyIndex]));

            UpdatePartyPanels();
            _Drawer->Render();
            return;
        }
        else if (keyCode == 77)  // 오른쪽 화살표
        {
            // 다음 파티원 선택 (빈 슬롯은 건너뛰기)
            int attempts = 0;
            do {
                _SelectedPartyIndex++;
                if (_SelectedPartyIndex > 3) _SelectedPartyIndex = 0;
                attempts++;
            } while (attempts < 4 && (_SelectedPartyIndex >= party.size() || !party[_SelectedPartyIndex]));

            UpdatePartyPanels();
            _Drawer->Render();
            return;
        }
        else if (keyCode == VK_RETURN || keyCode == VK_SPACE)  // Enter 또는 Space - 확정
        {
            if (_IsCancelMode)
            {
                // 취소 모드 - 선택된 파티원의 예약 취소
                Player* selectedPlayer = party[_SelectedPartyIndex].get();
                const auto& reservations = battleMgr->GetActiveReservations();

                int cancelCount = 0;
                for (const auto& res : reservations)
                {
                    // 예약 대상이 선택된 파티원인지 확인
                    if (res.IsActive && res.Target == selectedPlayer)
                    {
                        if (battleMgr->CancelItemReservation(mainPlayer, res.SlotIndex))
                        {
                            cancelCount++;
                        }
                    }
                }

                if (cancelCount > 0)
                {
                    _SystemLogs.push_back("[안내] " + selectedPlayer->GetName() + "의 " +
                        std::to_string(cancelCount) + "개 예약이 취소되었습니다.");
                }
                else
                {
                    _SystemLogs.push_back("[안내] " + selectedPlayer->GetName() + "의 예약이 없습니다.");
                }

                _IsCancelMode = false;
                _SelectedPartyIndex = 0;
            }
            else if (_IsSelectingItem)
            {
                // 아이템 선택 모드 - 예약 등록
                Inventory* inventory = nullptr;
                if (!mainPlayer->TryGetInventory(inventory) || !inventory)
                {
                    _SystemLogs.push_back("[오류] 인벤토리를 사용할 수 없습니다.");
                    _IsSelectingItem = false;
                    _SelectedPartyIndex = 0;
                }
                else
                {
                    IItem* item = inventory->GetItemAtSlot(_SelectedItemSlot);
                    if (!item)
                    {
                        _SystemLogs.push_back("[경고] 해당 슬롯에 아이템이 없습니다.");
                        _IsSelectingItem = false;
                        _SelectedPartyIndex = 0;
                    }
                    else
                    {
                        Player* targetPlayer = party[_SelectedPartyIndex].get();

                        // 이미 예약되어 있는지 확인
                        if (item->IsReserved())
                        {
                            // 예약 취소 후 재예약
                            if (battleMgr->CancelItemReservation(mainPlayer, _SelectedItemSlot))
                            {
                                if (battleMgr->ReserveItemUse(mainPlayer, targetPlayer, _SelectedItemSlot))
                                {
                                    _SystemLogs.push_back("[안내] " + item->GetName() + " 예약 대상을 " +
                                        targetPlayer->GetName() + "으로 변경했습니다.");
                                }
                                else
                                {
                                    _SystemLogs.push_back("[오류] 예약 변경 실패.");
                                }
                            }
                        }
                        else
                        {
                            // 새 예약
                            if (battleMgr->ReserveItemUse(mainPlayer, targetPlayer, _SelectedItemSlot))
                            {
                                _SystemLogs.push_back("[안내] " + item->GetName() + " 예약 완료! (대상: " +
                                    targetPlayer->GetName() + ", " + item->GetUseConditionDescription() + ")");
                            }
                            else
                            {
                                _SystemLogs.push_back("[오류] 예약 실패.");
                            }
                        }

                        _IsSelectingItem = false;
                        _SelectedPartyIndex = 0;
                    }
                }
            }

            Panel* logPanel = _Drawer->GetPanel("SystemLog");
            if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

            Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
            if (inventoryPanel) UpdateInventoryPanel(inventoryPanel);

            UpdatePartyPanels();
            _Drawer->Render();
            return;
        }
        else if (keyCode == VK_ESCAPE)  // ESC - 취소
        {
            _IsSelectingItem = false;
            _IsCancelMode = false;
            _SelectedPartyIndex = 0;

            UpdatePartyPanels();
            _Drawer->Render();
            return;
        }
    }

    // Space: 턴 진행
    if (keyCode == VK_SPACE)
    {
        switch (_InputState)
        {
        case EBattleInputState::Playing:
            ProcessBattleTurn();
            break;

        case EBattleInputState::ResultShown:
        {
            const BattleResult& result =
                BattleManager::GetInstance()->GetBattleResult();

            _SystemLogs.push_back("");

            if (result.Victory)
            {
                _SystemLogs.push_back("[승리] 전투에서 승리했습니다!");
                _SystemLogs.push_back("[보상] 경험치: " +
                    std::to_string(result.ExpGained) +
                    ", 골드: " +
                    std::to_string(result.GoldGained) + "G");

                if (!result.ItemName.empty())
                    _SystemLogs.push_back("[보상] 아이템 획득: " + result.ItemName);
            }
            else
            {
                _SystemLogs.push_back("[패배] 전투에서 패배했습니다...");
            }

            _SystemLogs.push_back("");
            _SystemLogs.push_back("[안내] Space 키를 눌러 전투를 종료합니다.");

            _InputState = EBattleInputState::EndWaiting;

            UpdateSystemLog(_Drawer->GetPanel("SystemLog"), _SystemLogs);
            _Drawer->Render();
            break;
        }
        case EBattleInputState::EndWaiting:
            EndBattle(BattleManager::GetInstance()->GetBattleResult().Victory);
            break;
        }
        return;
    }

    // 1~5: 아이템 선택 (대상 선택 모드 진입)
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

        // ===== 이미 예약된 아이템인지 확인 =====
        if (item->IsReserved())
        {
            // 예약 대상 찾기
            const auto& reservations = battleMgr->GetActiveReservations();
            std::string reservedTargetName = "알 수 없음";

            for (const auto& res : reservations)
            {
                if (res.IsActive && res.SlotIndex == slotIndex && res.Target)
                {
                    reservedTargetName = res.Target->GetName();
                    break;
                }
            }

            _SystemLogs.push_back("[안내] " + item->GetName() + "은(는) 이미 " +
                reservedTargetName + "에게 예약되어 있습니다.");

            Panel* logPanel = _Drawer->GetPanel("SystemLog");
            if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

            _Drawer->Render();
            return;
        }

        // ===== 예약되지 않은 아이템 → 대상 선택 모드 진입 =====
        _IsSelectingItem = true;
        _IsCancelMode = false;
        _SelectedItemSlot = slotIndex;
        _SelectedPartyIndex = 0;  // 첫 번째 파티원부터 시작

        // 첫 번째 유효한 파티원 찾기
        int attempts = 0;
        while (attempts < 4 && (_SelectedPartyIndex >= party.size() || !party[_SelectedPartyIndex]))
        {
            _SelectedPartyIndex++;
            if (_SelectedPartyIndex > 3) _SelectedPartyIndex = 0;
            attempts++;
        }

        _SystemLogs.push_back("[안내] " + item->GetName() + " 사용 대상을 선택하세요. (←→ 방향키, Enter/Space 확정, ESC 취소)");

        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

        UpdatePartyPanels();
        _Drawer->Render();
        return;
    }
    // C: 예약 취소 모드
    if (keyCode == 'C' || keyCode == 'c')
    {
        _IsCancelMode = true;
        _IsSelectingItem = false;
        _SelectedPartyIndex = 0;

        // 첫 번째 유효한 파티원 찾기
        int attempts = 0;
        while (attempts < 4 && (_SelectedPartyIndex >= party.size() || !party[_SelectedPartyIndex]))
        {
            _SelectedPartyIndex++;
            if (_SelectedPartyIndex > 3) _SelectedPartyIndex = 0;
            attempts++;
        }

        _SystemLogs.push_back("[안내] 예약 취소 대상을 선택하세요. (←→ 방향키, Enter/Space 확정, ESC 취소)");

        Panel* logPanel = _Drawer->GetPanel("SystemLog");
        if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

        UpdatePartyPanels();
        _Drawer->Render();
        return;
    }
}

// ===== 전투 로직 연동 =====

void BattleScene::ProcessBattleTurn()
{
    BattleManager* battleMgr = BattleManager::GetInstance();

    // 1. 전투 비활성 상태 방어
    if (!battleMgr->IsBattleActive())
    {
        _SystemLogs.push_back("[오류] 전투가 활성화되지 않았습니다.");
        if (auto logPanel = _Drawer->GetPanel("SystemLog"))
            UpdateSystemLog(logPanel, _SystemLogs);
        return;
    }

    // 2. 라운드 로그
    _SystemLogs.push_back(
        "[전투] === 라운드 " +
        std::to_string(battleMgr->GetCurrentRound() + 1) +
        " 시작 ==="
    );

    // 3. 턴 처리
    bool continuesBattle = battleMgr->ProcessBattleTurn();

    // 4. UI 갱신
    UpdatePartyPanels();
    UpdateMonsterInfoPanel();
    UpdateBattleInfoPanel();

    // 5. 전투 종료 감지 → 상태만 변경
    if (!continuesBattle)
    {
        const BattleResult& result = battleMgr->GetBattleResult();

        if (result.Victory)
        {
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[승리] 전투에서 승리했습니다!");
            // ===== BattleResult에서 보상 정보 조회 =====
     // CalculateReward는 EndBattle()에서 호출되므로 여기서는 표시하지 않음
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[안내] Space 키를 눌러 보상을 확인하세요.");
        }
        else
        {
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[패배] 전투에서 패배했습니다...");
            _SystemLogs.push_back("");
            _SystemLogs.push_back("[안내] Space 키를 눌러 계속하세요.");
        }

        _BattleEnd = true;
        _InputState = EBattleInputState::ResultShown;
    }

    // 6. 로그/UI 반영
    if (auto logPanel = _Drawer->GetPanel("SystemLog"))
        UpdateSystemLog(logPanel, _SystemLogs);

    if (auto inventoryPanel = _Drawer->GetPanel("Inventory"))
        UpdateInventoryPanel(inventoryPanel);

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
    }

    // ===== 2. BattleManager 정리 (보상 계산 포함) =====
    battleMgr->EndBattle();

    // ===== 3. 보상 로그 수집 및 표시 =====
    if (victory)
    {
        const BattleResult& result = battleMgr->GetBattleResult();

        _SystemLogs.push_back("");
        _SystemLogs.push_back("[보상] 경험치: " + std::to_string(result.ExpGained) +
            ", 골드: " + std::to_string(result.GoldGained) + "G");

        if (!result.ItemName.empty())
        {
            _SystemLogs.push_back("[보상] 아이템 획득: " + result.ItemName);
        }

        // ===== BattleManager 로그 수집 (보상 로그 포함) =====
        CollectBattleLogs();

        _SystemLogs.push_back("");
        _SystemLogs.push_back("[성공] 스테이지 클리어!");
    }

    // ===== 4. 보상 정보 최종 표시 =====
    Panel* logPanel = _Drawer->GetPanel("SystemLog");
    if (logPanel)
    {
        UpdateSystemLog(logPanel, _SystemLogs);
    }

    _Drawer->Render();

    // ===== 5. 사용자 입력 대기 (보상 확인용) =====
    Sleep(2000);  // 2초 대기 (보상 읽을 시간)

    _IsActive = false;
    Exit();

    // ===== 6. 씬 전환 =====
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

// ===== 헬퍼 함수 =====

WORD BattleScene::GetCharacterColor(int partyIndex) const
{
    switch (partyIndex)
    {
    case 0: return MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);
    case 1: return MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
    case 2: return MakeColorAttribute(ETextColor::LIGHT_MAGENTA, EBackgroundColor::BLACK);
    case 3: return MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK);
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

// ===== 전투 로그 수집 =====
void BattleScene::CollectBattleLogs()
{
    BattleManager* battleMgr = BattleManager::GetInstance();
    auto logs = battleMgr->ConsumeLogs();

    for (const auto& log : logs)
    {
        _SystemLogs.push_back(log.Message);
    }

    Panel* logPanel = _Drawer->GetPanel("SystemLog");
    if (logPanel)
        UpdateSystemLog(logPanel, _SystemLogs);
}

// ===== IBattleAnimationCallback 구현 =====

void BattleScene::SetPanelAnimation(const std::string& panelName,
    const std::string& animJsonFile,
    float duration)
{
    Panel* panel = _Drawer->GetPanel(panelName);
    if (!panel) return;

    // 애니메이션 로더
    auto animRenderer = std::make_unique<AsciiArtRenderer>();
    std::string animPath = DataManager::GetInstance()->GetAnimationsPath();

    if (animRenderer->LoadAnimationFromJson(animPath, animJsonFile))
    {
        animRenderer->SetAlignment(ArtAlignment::CENTER);
        animRenderer->StartAnimation();
        panel->SetContentRenderer(std::move(animRenderer));
        panel->Redraw();
        _Drawer->Render();

        // duration > 0이면 블로킹 대기
        if (duration > 0.0f)
        {
            Sleep(static_cast<DWORD>(duration * 1000));
        }
    }
}

void BattleScene::SetPanelArt(const std::string& panelName,
    const std::string& artTxtFile)
{
    Panel* panel = _Drawer->GetPanel(panelName);
    if (!panel) return;

    // 정적 아스키 아트 로더
    auto artRenderer = std::make_unique<AsciiArtRenderer>();
    std::string folderPath;

    // 패널 이름으로 폴더 자동 감지
    if (panelName == "Enemy")
    {
        folderPath = DataManager::GetInstance()->GetMonstersPath();
    }
    else if (panelName.find("CharArt") == 0)
    {
        folderPath = DataManager::GetInstance()->GetCharactersPath();
    }
    else
    {
        folderPath = DataManager::GetInstance()->GetAnimationsPath();
    }

    if (artRenderer->LoadFromFile(folderPath, artTxtFile))
    {
        artRenderer->SetAlignment(ArtAlignment::CENTER);
        panel->SetContentRenderer(std::move(artRenderer));
        panel->Redraw();
        _Drawer->Render();
    }
}

void BattleScene::UpdatePartyDisplay()
{
    UpdatePartyPanels();
    _Drawer->Render();
}

void BattleScene::UpdateMonsterDisplay()
{
    UpdateMonsterInfoPanel();
    UpdateBattleInfoPanel();
    _Drawer->Render();
}

void BattleScene::RefreshBattleUI()
{
    UpdatePartyPanels();
    UpdateMonsterInfoPanel();
    UpdateBattleInfoPanel();

    Panel* logPanel = _Drawer->GetPanel("SystemLog");
    if (logPanel) UpdateSystemLog(logPanel, _SystemLogs);

    Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
    if (inventoryPanel) UpdateInventoryPanel(inventoryPanel);

    UpdateCommandPanel();

    _Drawer->Render();
}
