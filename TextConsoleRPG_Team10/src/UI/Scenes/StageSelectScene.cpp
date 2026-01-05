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
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"
#include "../../../include/Item/Inventory.h"
#include "../../../include/Item/IItem.h"
#include <algorithm>

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

    // =============================================================================
    // StageManager에서 현재 선택 가능한 노드 가져오기
    // =============================================================================
    StageManager* stageMgr = StageManager::GetInstance();
    //auto availableNodes = stageMgr->GetAvailableNextNodes();

    // _CurrentNodes.clear();
    // int index = 0;
    // for (const auto* nodeData : availableNodes)
    // {
    //     StageNode uiNode;
    //     uiNode.Type = nodeData->Type;
    //     uiNode.Index = index++;
    //     
    //     // 노드 이름 설정
    //     switch (nodeData->Type)
    //     {
    //   case ENodeType::Battle:
    //    uiNode.Name = "전투 (" + nodeData->EnemyType + " x" + 
    //    std::to_string(nodeData->EnemyCount) + ")";
  //         break;
    //     case ENodeType::Elite:
    //   uiNode.Name = "엘리트 전투 (" + nodeData->EnemyType + " x" + 
    //            std::to_string(nodeData->EnemyCount) + ")";
    //         break;
    //     case ENodeType::Boss:
    //         uiNode.Name = "보스 전투";
    //         break;
    // case ENodeType::Event:
    //         uiNode.Name = "이벤트: " + nodeData->EventType;
    //  break;
    //     case ENodeType::Exit:
    // uiNode.Name = "다음 층으로";
    // break;
    //     default:
    //  uiNode.Name = "알 수 없음";
    //         break;
    //     }
    //
    //     _CurrentNodes.push_back(uiNode);
    // }
    //
    // // 현재 층 정보 가져오기
    // const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();
    // if (floorInfo)
    // {
    //     _CurrentStageLevel = floorInfo->Floor;
    // }

    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 타이틀 패널 (상단) =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 2, 1, 120, 5);
    titlePanel->SetBorder(true, ETextColor::LIGHT_YELLOW);

    auto titleText = std::make_unique<TextRenderer>();

    // TODO: 타이틀 동적 업데이트
    const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();
    std::string title = "[  " + std::to_string(floorInfo->Floor) + "층 - " + floorInfo->Description + "  ]";

    titleText->AddLineWithColor(title,
        MakeColorAttribute(ETextColor::BLACK, EBackgroundColor::WHITE));

    //titlePanel->SetContentRenderer(std::move(titleText));
    titlePanel->AddRenderer(50, 1, 100, 3, std::move(titleText));
    titlePanel->Redraw();

    // ===== 진행 안내 패널 (상단 중앙) =====
    Panel* guidePanel = _Drawer->CreatePanel("Guide", 2, 6, 120, 5);
    guidePanel->SetBorder(true, ETextColor::YELLOW);

    // TODO: 진행 현황 동적 업데이트
    const StageProgress& progress = stageMgr->GetProgress();
    std::string stats = "[토벌 현황] 전투: " + std::to_string(progress.TotalBattlesCompleted) + " 회, " +
        "일반: " + std::to_string(progress.NormalMonstersKilled) + " 마리, " +
        "엘리트: " + std::to_string(progress.EliteMonstersKilled) + " 마리";
    auto guideText = std::make_unique<TextRenderer>();
    guideText->AddLine("");
    guideText->AddLineWithColor(stats, MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    //guidePanel->SetContentRenderer(std::move(guideText));
    guidePanel->AddRenderer(35, 1, 100, 3, std::move(guideText));
    guidePanel->Redraw();

    // ===== 스테이지 진입 분할 노드 (중앙) =====
    Panel* nodePanel = _Drawer->CreatePanel("Nodes", 2, 11, 120, 20);
    nodePanel->SetBorder(true, ETextColor::WHITE);

    auto nodeText = std::make_unique<TextRenderer>();
    nodeText->AddLine("");
    nodeText->AddLine("");
    nodeText->AddLine("");
    nodeText->AddLine("     [스테이지 진입 분할 노드]");
    nodeText->AddLine("");
    nodeText->SetTextColor(MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    nodePanel->SetContentRenderer(std::move(nodeText));
    nodePanel->Redraw();

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
        errorText->AddLineWithColor("[ Title.txt not found ]", static_cast<WORD>(ETextColor::LIGHT_RED));
        errorText->AddLine("");
        errorText->SetTextColor(static_cast<WORD>(ETextColor::LIGHT_YELLOW));
        towerPanel->SetContentRenderer(std::move(errorText));
    }
    UpdateTowerArrow(towerPanel, floorInfo->Floor);

    // TODO: 노드 목록 동적 표시
    // Panel* nodePanel = _Drawer->GetPanel("Nodes");
    // auto nodeText = std::make_unique<TextRenderer>();
    //
    // nodeText->AddLine("");
    // nodeText->AddLine("  선택 가능한 경로:");
    // nodeText->AddLine("");
    //
    // for (size_t i = 0; i < _CurrentNodes.size(); ++i) {
    //     std::string nodeIcon = "";
    //     ETextColor nodeColor = ETextColor::WHITE;
    //
    //     switch (_CurrentNodes[i].Type) {
    //   case ENodeType::Battle:
    //         nodeIcon = "[전투]";
    //         nodeColor = ETextColor::LIGHT_RED;
    //         break;
    //     case ENodeType::Elite:
    //         nodeIcon = "[엘리트]";
    //         nodeColor = ETextColor::LIGHT_MAGENTA;
    //         break;
    //     case ENodeType::Event:
    //         nodeIcon = "[이벤트]";
  //     nodeColor = ETextColor::LIGHT_GREEN;
    //         break;
    //     case ENodeType::Boss:
 //         nodeIcon = "[보스]";
    //   nodeColor = ETextColor::YELLOW;
    //  break;
    //     case ENodeType::Exit:
    //   nodeIcon = "[다음층]";
    //         nodeColor = ETextColor::LIGHT_CYAN;
    //         break;
    //     }
    //
    //     std::string prefix = (i == _SelectedNodeIndex) ? "> " : "  ";
    //     std::string nodeLine = prefix + nodeIcon + " " + _CurrentNodes[i].Name;
    //
//     if (i == _SelectedNodeIndex) {
    //         nodeText->AddLineWithColor(nodeLine,
    //             MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
    //     } else {
    //         nodeText->AddLineWithColor(nodeLine,
    //         MakeColorAttribute(nodeColor, EBackgroundColor::BLACK));
    //     }
    //     nodeText->AddLine("");
    // }
    //
    // nodePanel->SetContentRenderer(std::move(nodeText));
    // nodePanel->Redraw();

    // ===== 진입 방식 및 키 설명 (하단) =====
    Panel* controlPanel = _Drawer->CreatePanel("Control", 2, 31, 120, 3);
    controlPanel->SetBorder(true, ETextColor::LIGHT_CYAN);

    auto controlText = std::make_unique<TextRenderer>();
    controlText->AddLineWithColor("  [진입 방식 및 키 설명]   [←/→/↑/↓] 선택   [Enter] 진입   [ESC] 메인 메뉴",
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    //controlPanel->SetContentRenderer(std::move(controlText));
    controlPanel->AddRenderer(20, 0, 100, 3, std::move(controlText));
    controlPanel->Redraw();

    // ===== 인벤토리 & 시스템 로그 패널 (하단 우측) =====
    Panel* systemPanel = _Drawer->CreatePanel("System", 2, 34, 100, 11);
    systemPanel->SetBorder(true, ETextColor::WHITE);

    // 시스템 로그 초기 메시지
    std::vector<std::string> initialLogs = {
        "[디버그] StageSelectScene 진입",
        "",
        "[정보] 다음 스테이지를 선택하세요.",
        "[성공] 게임을 시작합니다."
    };
    UpdateSystemLog(systemPanel, initialLogs);

    Panel* inventoryPanel = _Drawer->CreatePanel("Inventory", 102, 34, 47, 11);
    inventoryPanel->SetBorder(true, ETextColor::WHITE);

    // 인벤토리 초기 표시
    UpdateInventoryPanel(inventoryPanel);

    _Drawer->Render();
}

// 화살표 갱신 함수 (별도 분리)
void StageSelectScene::UpdateTowerArrow(Panel* towerPanel, int currentFloor)
{
    auto arrowRenderer = std::make_unique<TextRenderer>();

    // Tower.txt 높이가 30줄, 10층이라고 가정
    // 10층 = 상단(줄 2), 1층 = 하단(줄 26) 정도로 매핑
    const int towerHeight = 25;
    const int maxFloor = 10;
    const int topMargin = 6;      // 타워 상단 여백
    const int bottomMargin = 0;   // 타워 하단 여백
    const int usableHeight = towerHeight - topMargin - bottomMargin;

    // 층수를 세로 위치로 변환 (역순: 10층이 위, 1층이 아래)
    int arrowLine = topMargin + ((maxFloor - currentFloor) * usableHeight / maxFloor);

    // 화살표 위치까지 빈 줄 추가
    for (int i = 0; i < arrowLine; ++i)
    {
        arrowRenderer->AddLine("");
    }

    // 현재 층 화살표 표시
    arrowRenderer->AddLineWithColor("*----►",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    // 화살표 오버레이 (타워 왼쪽)
    towerPanel->AddRenderer(0, 0, 5, towerHeight, std::move(arrowRenderer));
    towerPanel->Redraw();
}

// 시스템 로그 업데이트 함수
// messages: 표시할 메시지 목록 (최신 메시지부터)
// 다른 씬에서 복사-붙여넣기 하여 사용 가능
void StageSelectScene::UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages)
{
    if (!systemPanel) return;

    auto logText = std::make_unique<TextRenderer>();
    logText->AddLineWithColor("[ 시스템 로그 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    // 최대 8줄까지만 표시 (패널 높이 11 - 헤더 1줄 - 여백 2줄)
    int maxLines = 8;
    int messageSize = static_cast<int>(messages.size());
    int displayCount = (messageSize < maxLines) ? messageSize : maxLines;

    for (int i = 0; i < displayCount; ++i)
    {
        // 빈 문자열은 그대로 빈 줄로 추가
        if (messages[i].empty())
        {
            logText->AddLine("");
            continue;
        }

        // 메시지 타입에 따라 색상 구분
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

    // SetContentRenderer 대신 AddRenderer 사용 (여백 제거)
    systemPanel->ClearRenderers();
    systemPanel->AddRenderer(0, 0, 98, 9, std::move(logText));
    systemPanel->Redraw();
}

// 인벤토리 패널 업데이트 함수
// 다른 씬에서 복사-붙여넣기 하여 사용 가능
void StageSelectScene::UpdateInventoryPanel(Panel* inventoryPanel)
{
    if (!inventoryPanel) return;

    auto inventoryText = std::make_unique<TextRenderer>();

    // 플레이어 인벤토리 가져오기
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

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory) || !inventory)
    {
        inventoryText->AddLineWithColor("[ 인벤토리 ]",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        inventoryText->AddLineWithColor("인벤토리가 비활성화되어 있습니다.",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    // 사용 중인 슬롯 개수 계산
    int usedSlots = 0;
    const int maxSlots = 5;  // 최대 5개 슬롯
    for (int i = 0; i < maxSlots; ++i)
    {
        if (inventory->GetItemAtSlot(i) != nullptr)
            usedSlots++;
    }

    // 헤더 (사용 중인 슬롯 / 전체 슬롯)
    std::string header = "[ 인벤토리 (" + std::to_string(usedSlots) + "/" + std::to_string(maxSlots) + ") ]";
    inventoryText->AddLineWithColor(header,
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    // 인벤토리 슬롯 최대 5개만 표시
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

    // SetContentRenderer 대신 AddRenderer 사용 (여백 제거)
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

void StageSelectScene::HandleInput()
{
    // =============================================================================
    // 입력 처리 구현 예시
    // =============================================================================
    //
    // InputManager* input = InputManager::GetInstance();
    // if (!input->IsKeyPressed()) return;
    //
    // int keyCode = input->GetKeyCode();
    //
    // // ===== 위/아래 키로 노드 선택 =====
    // if (keyCode == VK_UP)  // ↑ 위로
    // {
    //     _SelectedNodeIndex--;
    //     if (_SelectedNodeIndex < 0)
    //         _SelectedNodeIndex = static_cast<int>(_CurrentNodes.size()) - 1;
    //
    //     // 노드 패널 업데이트 (위의 TODO 참고)
    //     _Drawer->Render();
    // }
    // else if (keyCode == VK_DOWN)  // ↓ 아래로
    // {
    //     _SelectedNodeIndex++;
    //     if (_SelectedNodeIndex >= static_cast<int>(_CurrentNodes.size()))
    //         _SelectedNodeIndex = 0;
    //
    //     // 노드 패널 업데이트
    //     _Drawer->Render();
    // }
    //
    // // ===== Enter 키로 노드 진입 =====
    // else if (keyCode == VK_RETURN)  // Enter - 노드 진입
    // {
    //     if (_SelectedNodeIndex >= 0 && _SelectedNodeIndex < _CurrentNodes.size())
    //     {
    //         StageManager* stageMgr = StageManager::GetInstance();
    //       auto availableNodes = stageMgr->GetAvailableNextNodes();
    //
    //         if (_SelectedNodeIndex < availableNodes.size())
    //         {
    //       const NodeData* selectedNode = availableNodes[_SelectedNodeIndex];
    //
    //        // 노드로 이동 (StageManager에 기록)
    //  if (stageMgr->MoveToNode(selectedNode->Id))
    //          {
    //        _IsActive = false;
    //       Exit();
    //
    //// ===== 노드 타입에 따라 Scene 전환 =====
    //         switch (selectedNode->Type)
    //   {
    //      case ENodeType::Battle:
  //    case ENodeType::Elite:
    //   // 일반/엘리트 전투 시작
    //          BattleManager::GetInstance()->StartBattle(EBattleType::Normal);
    //        SceneManager::GetInstance()->ChangeScene(ESceneType::Battle);
    //       break;
    //
    //        case ENodeType::Boss:
    //         // 보스 전투 시작
    //          BattleManager::GetInstance()->StartBattle(EBattleType::Boss);
    //       SceneManager::GetInstance()->ChangeScene(ESceneType::Battle);
    //         break;
    //
    //       case ENodeType::Event:
    //           // 이벤트 타입에 따라 분기
//                if (selectedNode->EventType == "Companion")
    //         {
    // SceneManager::GetInstance()->ChangeScene(ESceneType::CompanionRecruit);
  //             }
    //        // 다른 이벤트 타입 추가 가능
    //          break;
    //
    //       case ENodeType::Exit:
    //      // 다음 층으로 이동
    //          if (stageMgr->MoveToNextFloor())
    //   {
    //    // 성공: 다시 StageSelect로 (새 층의 노드 표시)
    //    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
    //      }
    //         else
    //         {
  //   // 실패 (10층 초과): 게임 클리어 - 결과 화면으로
    //          SceneManager::GetInstance()->ChangeScene(ESceneType::Result);
    //         }
    //          break;
    //
    //        default:
    //        break;
// }
    //}
    //         }
    //     }
    // }
    //
    // // ===== ESC 키로 메인 메뉴 복귀 (로그라이크이므로 진행 포기) =====
    // else if (keyCode == VK_ESCAPE)  // ESC - 메인 메뉴
    // {
    //     _IsActive = false;
    //     Exit();
    //     SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
    // }
}
