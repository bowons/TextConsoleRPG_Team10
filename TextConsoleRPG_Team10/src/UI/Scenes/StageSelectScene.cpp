#include "../../../include/UI/Scenes/StageSelectScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/StageManager.h"
#include "../../../include/Manager/BattleManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/SoundPlayer.h"
#include "../../../include/Common/TextColor.h"
#include "../../../include/UI/Scenes/ResultScene.h"  // 추가
#include "../../../include/Unit/Player.h"
#include "../../../include/Item/Inventory.h"
#include "../../../include/Item/IItem.h"
#include <algorithm>
#include <set>

StageSelectScene::StageSelectScene()
    : UIScene("StageSelect")
    , _SelectedNodeIndex(0)
    , _CurrentStageLevel(1)
{
}

StageSelectScene::~StageSelectScene()
{
}

void StageSelectScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;
    _SelectedNodeIndex = 0;

    // 입력 버퍼 완전히 클리어 (이전 Scene의 입력 잔여물 제거)
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hInput);

    StageManager* stageMgr = StageManager::GetInstance();
    const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();

    // ===== 선택 가능한 노드 초기화 =====
    RefreshAvailableNodes();

    // ===== 타이틀 패널 (상단) =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 2, 1, 120, 5);
    titlePanel->SetBorder(true, ETextColor::WHITE);

    auto titleText = std::make_unique<TextRenderer>();
    std::string title = "[  " + std::to_string(floorInfo->Floor) + "층 - " + floorInfo->Description + "  ]";
    titleText->AddLineWithColor(title,
        MakeColorAttribute(ETextColor::BLACK, EBackgroundColor::WHITE));

    titlePanel->AddRenderer(50, 1, 100, 3, std::move(titleText));
    titlePanel->Redraw();

    // ===== 진행 안내 패널 (상단 중앙) =====
    Panel* guidePanel = _Drawer->CreatePanel("Guide", 2, 6, 120, 5);
    guidePanel->SetBorder(true, ETextColor::WHITE);

    UpdateGuidePanel(guidePanel);

    // ===== 스테이지 진입 분할 노드 (중앙) =====
    Panel* nodePanel = _Drawer->CreatePanel("Nodes", 2, 11, 120, 20);
    nodePanel->SetBorder(true, ETextColor::WHITE);

    RenderStageMap(nodePanel);

    // ===== 타워 패널 (우측) =====
    Panel* towerPanel = _Drawer->CreatePanel("Tower", 122, 1, 30, 30);
    auto towerArt = std::make_unique<AsciiArtRenderer>();
    std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");

    bool towerLoaded = towerArt->LoadFromFile(uiPath, "Tower.txt");

    if (towerLoaded)
    {
        towerArt->SetAlignment(ArtAlignment::CENTER);
        towerArt->SetColor(ETextColor::WHITE);
        towerPanel->SetContentRenderer(std::move(towerArt));
    }
    else
    {
        auto errorText = std::make_unique<TextRenderer>();
        errorText->AddLine("");
        errorText->AddLine("");
        errorText->AddLineWithColor("[ Tower.txt not found ]", static_cast<WORD>(ETextColor::LIGHT_RED));
        errorText->AddLine("");
        errorText->SetTextColor(static_cast<WORD>(ETextColor::LIGHT_YELLOW));
        towerPanel->SetContentRenderer(std::move(errorText));
    }
    UpdateTowerArrow(towerPanel, floorInfo->Floor);

    // ===== 진입 방식 및 키 설명 (하단) =====
    Panel* controlPanel = _Drawer->CreatePanel("Control", 2, 31, 120, 3);
    controlPanel->SetBorder(true, ETextColor::WHITE);

    auto controlText = std::make_unique<TextRenderer>();
    controlText->AddLineWithColor("  [진입 방식 및 키 설명]   [←/→/↑/↓] 선택   [Enter] 진입   [Space] 상점   [ESC] 메인 메뉴",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    controlPanel->AddRenderer(10, 0, 100, 3, std::move(controlText));
    controlPanel->Redraw();

    // ===== 인벤토리 & 시스템 로그 패널 (하단 우측) =====
    Panel* systemPanel = _Drawer->CreatePanel("System", 2, 34, 100, 11);
    systemPanel->SetBorder(true, ETextColor::WHITE);

    std::vector<std::string> initialLogs = {
        "[디버그] StageSelectScene 진입",
        "",
        "[정보] 다음 스테이지를 선택하세요.",
        "[성공] 게임을 시작합니다."
    };
    UpdateSystemLog(systemPanel, initialLogs);

    Panel* inventoryPanel = _Drawer->CreatePanel("Inventory", 102, 34, 47, 11);
    inventoryPanel->SetBorder(true, ETextColor::WHITE);

    UpdateInventoryPanel(inventoryPanel);

    _Drawer->Render();
}

void StageSelectScene::UpdateTowerArrow(Panel* towerPanel, int currentFloor)
{
    auto arrowRenderer = std::make_unique<TextRenderer>();

    const int towerHeight = 25;
    const int maxFloor = 10;
    const int topMargin = 6;
    const int bottomMargin = 0;
    const int usableHeight = towerHeight - topMargin - bottomMargin;

    int arrowLine = topMargin + ((maxFloor - currentFloor) * usableHeight / maxFloor);

    for (int i = 0; i < arrowLine; ++i)
    {
        arrowRenderer->AddLine("");
    }

    arrowRenderer->AddLineWithColor("*----►",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    towerPanel->AddRenderer(0, 0, 5, towerHeight, std::move(arrowRenderer));
    towerPanel->Redraw();
}

void StageSelectScene::UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages)
{
    if (!systemPanel) return;

    systemPanel->ClearRenderers();
    systemPanel->SetDirty();

    auto logText = std::make_unique<TextRenderer>();
    logText->AddLineWithColor("[ 시스템 로그 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    int maxLines = 8;
    int messageSize = static_cast<int>(messages.size());
    int displayCount = (messageSize < maxLines) ? messageSize : maxLines;

    for (int i = 0; i < displayCount; ++i)
    {
        if (messages[i].empty())
        {
            logText->AddLine("");
            continue;
        }

        WORD color;
        if (messages[i].find("[디버그]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK);
        else if (messages[i].find("[경고]") != std::string::npos || messages[i].find("[오류]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK);
        else if (messages[i].find("[성공]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
        else if (messages[i].find("[정보]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);
        else
            color = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

        logText->AddLineWithColor(messages[i], color);
    }

    systemPanel->ClearRenderers();
    systemPanel->AddRenderer(0, 0, 98, 9, std::move(logText));
    systemPanel->Redraw();
}

void StageSelectScene::UpdateInventoryPanel(Panel* inventoryPanel)
{
    if (!inventoryPanel) return;

    auto inventoryText = std::make_unique<TextRenderer>();

    Player* player = GameManager::GetInstance()->GetMainPlayer().get();

    if (!player)
    {
        inventoryText->AddLineWithColor("[ 인벤토리 ]",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        inventoryText->AddLineWithColor("플레이어 정보를 불러올 수 없습니다.",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    std::string goldInfo = "[ 소지금: " + std::to_string(player->GetGold()) + " G ]";
    inventoryText->AddLineWithColor(goldInfo,
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory) || !inventory)
    {
        inventoryText->AddLineWithColor("인벤토리가 비활성화되어 있습니다.",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    int usedSlots = 0;
    const int maxSlots = 5;
    for (int i = 0; i < maxSlots; ++i)
    {
        if (inventory->GetItemAtSlot(i) != nullptr)
            usedSlots++;
    }

    std::string header = "[ 인벤토리 (" + std::to_string(usedSlots) + "/" + std::to_string(maxSlots) + ") ]";
    inventoryText->AddLineWithColor(header,
        MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));

    for (int i = 0; i < maxSlots; ++i)
    {
        IItem* item = inventory->GetItemAtSlot(i);
        if (item)
        {
            int amount = inventory->GetSlotAmount(i);
            std::string itemLine = std::to_string(i + 1) + ". " +
                item->GetName() + " x" + std::to_string(amount);
            inventoryText->AddLineWithColor(itemLine,
                MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        }
        else
        {
            std::string emptyLine = std::to_string(i + 1) + ". [빈 슬롯]";
            inventoryText->AddLineWithColor(emptyLine,
                MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        }
    }

    inventoryPanel->ClearRenderers();
    inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
    inventoryPanel->Redraw();
}

void StageSelectScene::Exit()
{
    _Drawer->RemoveAllPanels();
    _CurrentNodes.clear();
    _IsActive = false;
}

void StageSelectScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void StageSelectScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

std::string StageSelectScene::GetNodeIcon(ENodeType type) const
{
    switch (type)
    {
    case ENodeType::Start:  return "[★]";
    case ENodeType::Battle: return "[ N ]";
    case ENodeType::Elite:  return "[ E ]";
    case ENodeType::Boss:   return "[ B ]";
    case ENodeType::Event:  return "[ ? ]";
    case ENodeType::Empty:  return "[ - ]";
    case ENodeType::Exit:   return "[UP]";
    default: return "[?]";
    }
}

std::string StageSelectScene::GetNodeIcon(const NodeData* node) const
{
    if (!node) return "[?]";

    StageManager* stageMgr = StageManager::GetInstance();

    // ===== 완료된 노드는 [ - ] 표시 =====
    if (stageMgr->IsNodeCompleted(node->Id))
    {
        return "[ - ]";
    }

    if (node->Type == ENodeType::Battle)
    {
        if (node->EnemyType == "Elite")
            return "[ E ]";
        else if (node->EnemyType == "Boss")
            return "[ B ]";
        else
            return "[ N ]";
    }

    switch (node->Type)
    {
    case ENodeType::Start:  return "[★]";
    case ENodeType::Event:  return "[ ? ]";
    case ENodeType::Empty:  return "[ - ]";
    case ENodeType::Exit:   return "[UP]";
    default:     return "[?]";
    }
}

WORD StageSelectScene::GetNodeColor(const NodeData* node) const
{
    if (!node) return MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

    // 기본적으로 모든 노드는 흰색으로 표시 (선택된 노드만 강조 표시)
    return MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);
}

void StageSelectScene::UpdateGuidePanel(Panel* guidePanel)
{
    if (!guidePanel) return;

    auto guideText = std::make_unique<TextRenderer>();

    StageManager* stageMgr = StageManager::GetInstance();
    const StageProgress& progress = stageMgr->GetProgress();

    std::string stats = "[토벌 현황] 전투: " + std::to_string(progress.TotalBattlesCompleted) + " 회, " +
        "일반: " + std::to_string(progress.NormalMonstersKilled) + " 마리, " +
        "엘리트: " + std::to_string(progress.EliteMonstersKilled) + " 마리";

    guideText->AddLineWithColor(stats, MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    std::string legend = "[★] 현재 위치   [N] 일반   [E] 엘리트   [B] 보스   [?] 이벤트   [-] 빈 노드   [UP] 다음 층";
    guideText->AddLineWithColor(legend, MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    guidePanel->ClearRenderers();
    guidePanel->AddRenderer(2, 0, 116, 4, std::move(guideText));
    guidePanel->Redraw();
}

void StageSelectScene::RenderStageMap(Panel* nodePanel)
{
    if (!nodePanel) return;

    StageManager* stageMgr = StageManager::GetInstance();
    const auto& allNodes = stageMgr->GetCurrentFloorNodes();
    const NodeData* currentNode = stageMgr->GetCurrentNode();

    if (allNodes.empty())
    {
        auto errorText = std::make_unique<TextRenderer>();
        errorText->AddLineWithColor("맵 데이터를 불러올 수 없습니다.",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        nodePanel->SetContentRenderer(std::move(errorText));
        nodePanel->Redraw();
        return;
    }

    auto mapText = std::make_unique<TextRenderer>();

    const int horizontalSpacing = 15;  // 노드 간 수평 간격 (연결선 길이)
    const int verticalSpacing = 2;     // 노드 간 수직 간격
    const int mapHeight = 18;
    const int mapWidth = 116;          // 패널 너비 (120 - 테두리 2칸)
    const int nodeWidth = 7;       // 통일된 노드 너비

    std::vector<std::string> mapLines(mapHeight, std::string(mapWidth, ' '));

    // 맵의 실제 범위 계산
    int minPosX = 0, maxPosX = 0;
    int minPosY = 0, maxPosY = 0;

    for (const auto& node : allNodes)
    {
        if (node.PosX < minPosX) minPosX = node.PosX;
        if (node.PosX > maxPosX) maxPosX = node.PosX;
        if (node.PosY < minPosY) minPosY = node.PosY;
        if (node.PosY > maxPosY) maxPosY = node.PosY;
    }

    // 맵의 실제 크기 계산
    int mapRealWidth = (maxPosX - minPosX) * horizontalSpacing + nodeWidth;
    int mapRealHeight = (maxPosY - minPosY) * verticalSpacing + 1;

    // 중앙 정렬을 위한 오프SET 계산
    int baseX = (mapWidth - mapRealWidth) / 2;
    int baseY = (mapHeight - mapRealHeight) / 2;

    // 최소 여백 보장
    if (baseX < 5) baseX = 5;
    if (baseY < 2) baseY = 2;

    int centerY = baseY + (mapRealHeight / 2);

    struct NodePosition
    {
        const NodeData* node;
        int screenX;
        int screenY;
        bool isCurrent;
        bool isSelected;
        bool isCompleted;
    };

    std::vector<NodePosition> nodePositions;

    for (const auto& node : allNodes)
    {
        // minPosX를 기준으로 상대 위치 계산 (왼쪽 정렬 방지)
        int screenX = baseX + ((node.PosX - minPosX) * horizontalSpacing);
        int screenY = centerY + (node.PosY * verticalSpacing);

        if (screenY >= 0 && screenY < mapHeight && screenX >= 0 && screenX + nodeWidth <= mapWidth)
        {
            bool isCurrent = currentNode && (node.Id == currentNode->Id);
            bool isSelected = IsNodeSelected(node.Id);
            bool isCompleted = stageMgr->IsNodeCompleted(node.Id);

            nodePositions.push_back({ &node, screenX, screenY, isCurrent, isSelected, isCompleted });
        }
    }

    // 연결선 그리기
    for (const auto& nodePos : nodePositions)
    {
        for (const auto& connId : nodePos.node->Connections)
        {
            auto it = std::find_if(nodePositions.begin(), nodePositions.end(),
                [&connId](const NodePosition& np) { return np.node->Id == connId; });

            if (it != nodePositions.end())
            {
                int x1 = nodePos.screenX;
                int y1 = nodePos.screenY;
                int x2 = it->screenX;
                int y2 = it->screenY;

                int iconEnd = x1 + nodeWidth;
                int iconCenter = x1 + (nodeWidth / 2);  // 노드 중앙 위치

                // 수평선
                if (y1 == y2)
                {
                    int startX = iconEnd;
                    int endX = x2;

                    if (startX < endX)
                    {
                        for (int x = startX; x < endX && x < mapWidth; ++x)
                        {
                            if (mapLines[y1][x] == ' ')
                                mapLines[y1][x] = '-';
                            else if (mapLines[y1][x] == '|')
                                mapLines[y1][x] = '+';
                        }
                    }
                }
                // 수직선 + 수평선
                else
                {
                    int direction = (y2 > y1) ? 1 : -1;

                    // 수직선 (노드 중앙에서 시작)
                    for (int y = y1 + direction; y != y2; y += direction)
                    {
                        if (y >= 0 && y < mapHeight && iconCenter < mapWidth)
                        {
                            if (mapLines[y][iconCenter] == ' ')
                                mapLines[y][iconCenter] = '|';
                            else if (mapLines[y][iconCenter] == '-')
                                mapLines[y][iconCenter] = '+';
                        }
                    }

                    // 수평선 (수직선 끝에서 목표 노드까지)
                    int horizontalStartX = iconCenter;
                    int horizontalEndX = x2;

                    if (horizontalStartX < horizontalEndX)
                    {
                        for (int x = horizontalStartX; x < horizontalEndX && x < mapWidth; ++x)
                        {
                            if (mapLines[y2][x] == ' ')
                                mapLines[y2][x] = '-';
                            else if (mapLines[y2][x] == '|')
                                mapLines[y2][x] = '+';
                        }
                    }
                }
            }
        }
    }

    // 노드 아이콘 삽입
    std::set<int> linesWithExit;

    for (const auto& nodePos : nodePositions)
    {
        std::string icon = GetNodeIcon(nodePos.node);
        int x = nodePos.screenX;
        int y = nodePos.screenY;

        if (y >= 0 && y < mapHeight)
        {
            // Exit 중복 방지
            if (nodePos.node->Type == ENodeType::Exit)
            {
                if (linesWithExit.find(y) != linesWithExit.end())
                    continue;
                linesWithExit.insert(y);
            }

            // ===== 현재 위치 표시 우선 =====
            std::string displayIcon;
            if (nodePos.isCurrent)
            {
                displayIcon = " [★] ";  // 현재 위치 강조
            }
            else if (nodePos.isSelected)
            {
                displayIcon = ">" + icon + "<";  // >[N]<
            }
            else
            {
                displayIcon = " " + icon + " ";  //  [N]  
            }

            if (x + displayIcon.length() <= static_cast<size_t>(mapWidth))
            {
                mapLines[y].replace(x, displayIcon.length(), displayIcon);
            }
        }
    }

    // 각 줄을 TextRenderer에 추가
    for (const std::string& line : mapLines)
    {
        bool hasContent = false;
        for (char c : line)
        {
            if (c != ' ')
            {
                hasContent = true;
                break;
            }
        }

        if (!hasContent)
        {
            mapText->AddLine(line);
        }
        else
        {
            mapText->AddLineWithColor(line,
                MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
        }
    }

    nodePanel->SetContentRenderer(std::move(mapText));
    nodePanel->Redraw();
}

void StageSelectScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;

    int keyCode = input->GetKeyCode();

    if (keyCode == 0 || keyCode < 0) return;

    StageManager* stageMgr = StageManager::GetInstance();

    if (keyCode == VK_ESCAPE)
    {
    StageManager* stageMgr = StageManager::GetInstance();
 
   // ⭐ 게임 시작 직후 (1층, 전투 없음) → 메인 메뉴로
        if (stageMgr->GetCurrentFloor() == 1 && 
  stageMgr->GetProgress().TotalBattlesCompleted == 0)
  {
  std::vector<std::string> logs = { 
     "[정보] 메인 메뉴로 돌아갑니다." 
   };
  Panel* systemPanel = _Drawer->GetPanel("System");
          if (systemPanel) UpdateSystemLog(systemPanel, logs);
 
         _IsActive = false;
   Exit();
   GameManager::GetInstance()->ResetGame();
      SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
      }
   else
 {
            // ⭐ 게임 진행 중 → ESC 키 완전히 무시 (아무 동작 안 함)
   std::vector<std::string> logs = { 
    "[경고] 게임 진행 중에는 ESC를 사용할 수 없습니다.",
          "[안내] Space키로 상점을 이용하세요."
      };
 Panel* systemPanel = _Drawer->GetPanel("System");
   if (systemPanel) UpdateSystemLog(systemPanel, logs);
      
     // ⭐ 아무 동작도 하지 않음 (return만 하고 씬 종료 안 함)
  }
return;
    }

    // ===== Space: 상점으로 이동 =====
    if (keyCode == VK_SPACE)
    {
        std::vector<std::string> logs = {
       "[정보] 상점으로 이동합니다.",
       "[안내] ESC를 눌러 다시 돌아올 수 있습니다."
        };
        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel) UpdateSystemLog(systemPanel, logs);

        _IsActive = false;
        Exit();
        SceneManager::GetInstance()->ChangeScene(ESceneType::Shop);
        return;
    }

    // ===== Enter: 노드 진입=====
    if (keyCode == VK_RETURN)
    {
        if (!_SelectedNodeId.empty())
        {
            EnterNode(_SelectedNodeId);
            SoundPlayer::GetInstance()->PlaySFX("Footstep" + std::to_string(rand() % 9 + 1));
        }
        return;
    }

    // ===== 방향키: 노드 선택 =====
    if (_AvailableNodeIds.empty())
    {
        // 디버깅: 선택 가능한 노드가 없음
        std::vector<std::string> debugLogs;
        debugLogs.push_back("[경고] 선택 가능한 노드가 없어서 방향키 입력을 처리할 수 없습니다.");

        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel)
        {
            UpdateSystemLog(systemPanel, _SystemLogs);
        }
        return;
    }

    SoundPlayer::GetInstance()->PlaySFX("FootStep_" + std::to_string(rand() % 9 + 1));

    int currentIndex = -1;
    for (size_t i = 0; i < _AvailableNodeIds.size(); ++i)
    {
        if (_AvailableNodeIds[i] == _SelectedNodeId)
        {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    if (currentIndex == -1)
    {
        currentIndex = 0;
        _SelectedNodeId = _AvailableNodeIds[0];
    }

    int newIndex = currentIndex;

    if (keyCode == 72 || keyCode == 75)
    {
        newIndex--;
        if (newIndex < 0)
            newIndex = static_cast<int>(_AvailableNodeIds.size()) - 1;
    }
    else if (keyCode == 80 || keyCode == 77)
    {
        newIndex++;
        if (newIndex >= static_cast<int>(_AvailableNodeIds.size()))
            newIndex = 0;
    }
    else
    {
        return;
    }

    if (newIndex != currentIndex)
    {
        SelectNode(_AvailableNodeIds[newIndex]);
    }
}

bool StageSelectScene::IsNodeSelected(const std::string& nodeId) const
{
    return _SelectedNodeId == nodeId;
}

void StageSelectScene::RefreshAvailableNodes()
{
    StageManager* stageMgr = StageManager::GetInstance();
    const NodeData* currentNode = stageMgr->GetCurrentNode();

    if (!currentNode)
    {
        _AvailableNodeIds.clear();
        _SelectedNodeId = "";
        return;
    }

    // ===== 현재 노드의 연결된 노드들을 선택 가능 목록으로 설정 =====
    _AvailableNodeIds.clear();

    for (const auto& connId : currentNode->Connections)
    {
        const NodeData* connNode = stageMgr->FindNodeById(connId);
        if (connNode)
        {
            _AvailableNodeIds.push_back(connId);
        }
    }

    // ===== 선택된 노드 초기화 =====
    if (!_AvailableNodeIds.empty())
    {
        // 이전에 선택했던 노드가 연결 목록에 있으면 유지
        bool found = false;
        for (const auto& id : _AvailableNodeIds)
        {
            if (id == _SelectedNodeId)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            // 이전 선택이 없거나 연결되지 않았으면 첫 번째 노드 선택
            _SelectedNodeId = _AvailableNodeIds[0];
        }

        // 디버깅: 선택 가능한 노드 확인
        std::vector<std::string> debugLogs;
        debugLogs.push_back("[디버그] 선택 가능한 노드: " + std::to_string(_AvailableNodeIds.size()) + "개");
        for (const auto& id : _AvailableNodeIds)
        {
            const NodeData* node = stageMgr->FindNodeById(id);
            if (node)
            {
                std::string icon = GetNodeIcon(node);
                debugLogs.push_back("  - " + id + " " + icon);
            }
        }

        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel)
        {
            UpdateSystemLog(systemPanel, debugLogs);
        }
    }
    else
    {
        _SelectedNodeId = "";

        // 디버깅: 선택 가능한 노드가 없음
        std::vector<std::string> debugLogs;
        debugLogs.push_back("[경고] 현재 노드에서 연결된 노드가 없습니다!");

        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel)
        {
            UpdateSystemLog(systemPanel, debugLogs);
        }
    }
}

void StageSelectScene::EnterNode(const std::string& nodeId)
{
    if (nodeId.empty()) return;

    StageManager* stageMgr = StageManager::GetInstance();
    const NodeData* node = stageMgr->FindNodeById(nodeId);

    if (!node)
    {
        std::vector<std::string> logs = { "[오류] 노드를 찾을 수 없습니다." };
        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel) UpdateSystemLog(systemPanel, logs);
        return;
    }

    if (!stageMgr->MoveToNode(nodeId))
    {
        std::vector<std::string> logs = { "[경고] 해당 노드로 이동할 수 없습니다." };
        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel) UpdateSystemLog(systemPanel, logs);
        return;
    }

    _IsActive = false;
    Exit();

    SceneManager* sceneMgr = SceneManager::GetInstance();

    switch (node->Type)
    {
    case ENodeType::Battle:
        if (node->EnemyType == "Boss")
        {
            BattleManager::GetInstance()->StartBattle(EBattleType::Boss, stageMgr->GetCurrentFloor());
        }
        else if (node->EnemyType == "Elite")
        {
            BattleManager::GetInstance()->StartBattle(EBattleType::Elite, stageMgr->GetCurrentFloor());
        }
        else
        {
            BattleManager::GetInstance()->StartBattle(EBattleType::Normal, stageMgr->GetCurrentFloor());
        }
        sceneMgr->ChangeScene(ESceneType::Battle);
        break;

    case ENodeType::Event:
        if (node->EventType == "Companion")
        {
            sceneMgr->ChangeScene(ESceneType::CompanionRecruit);
        }
        break;

    case ENodeType::Exit:
    {
StageManager* stageMgr = StageManager::GetInstance();
     SceneManager* sceneMgr = SceneManager::GetInstance();
 
        if (stageMgr->GetCurrentFloor() >= 10)
  {
// ⭐ 10층 클리어 → 굿엔딩 (Floor 12 설정)
 stageMgr->SetCurrentFloor(12);  // ⭐ Public 메서드 사용
         
 std::vector<std::string> logs = { 
       "[성공] 10층 클리어! 엔딩으로 이동합니다." 
  };
 Panel* systemPanel = _Drawer->GetPanel("System");
     if (systemPanel) UpdateSystemLog(systemPanel, logs);
    
    sceneMgr->ChangeScene(ESceneType::StoryProgress);
        }
        else if (stageMgr->MoveToNextFloor())
        {
       // 일반 층 전환 (1~9층)
     sceneMgr->ChangeScene(ESceneType::StoryProgress);
  }
     else
        {
   // MoveToNextFloor 실패 시 (비정상 케이스)
    std::vector<std::string> logs = { 
      "[오류] 다음 층으로 이동할 수 없습니다." 
    };
            Panel* systemPanel = _Drawer->GetPanel("System");
if (systemPanel) UpdateSystemLog(systemPanel, logs);
  
        sceneMgr->ChangeScene(ESceneType::MainMenu);
      }
  break;
 }

    case ENodeType::Empty:
        // Empty 노드는 즉시 통과 → StageSelect 재진입으로 UI 갱신
        sceneMgr->ChangeScene(ESceneType::StageSelect);
        break;

    default:
        // 알 수 없는 노드 타입 → StageSelect 유지
        sceneMgr->ChangeScene(ESceneType::StageSelect);
        break;
    }
}

// ===== SelectNode: 노드 선택 업데이트 및 맵 재렌더링 =====
void StageSelectScene::SelectNode(const std::string& nodeId)
{
    if (nodeId.empty()) return;

    // 선택된 노드 ID 갱신
    _SelectedNodeId = nodeId;

    // 맵 패널 재렌더링
    Panel* nodePanel = _Drawer->GetPanel("Nodes");
    if (nodePanel)
    {
        RenderStageMap(nodePanel);
    }

    // 시스템 로그 업데이트 (선택된 노드 정보 표시)
    StageManager* stageMgr = StageManager::GetInstance();
    const NodeData* node = stageMgr->FindNodeById(nodeId);

    if (node)
    {
        std::vector<std::string> logs;
        logs.push_back("[선택] " + nodeId + " " + GetNodeIcon(node));

        std::string nodeInfo;
        if (node->Type == ENodeType::Battle)
        {
            if (node->EnemyType == "Boss")
                nodeInfo = "  → 보스 전투";
            else if (node->EnemyType == "Elite")
                nodeInfo = "  → 엘리트 전투";
            else
                nodeInfo = "  → 일반 전투";
        }
        else if (node->Type == ENodeType::Event)
        {
            if (node->EventType == "Companion")
                nodeInfo = "  → 동료 조우 이벤트";
            else
                nodeInfo = "  → 이벤트";
        }
        else if (node->Type == ENodeType::Exit)
        {
            nodeInfo = "  → 다음 층으로 이동";
        }
        else if (node->Type == ENodeType::Empty)
        {
            nodeInfo = "  → 빈 노드 (즉시 통과)";
        }

        if (!nodeInfo.empty())
            logs.push_back(nodeInfo);

        Panel* systemPanel = _Drawer->GetPanel("System");
        if (systemPanel)
        {
            UpdateSystemLog(systemPanel, logs);
        }
    }
}
