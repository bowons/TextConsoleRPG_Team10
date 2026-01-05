# StageSelectScene 구현 가이드

## 📋 개요

StageSelectScene은 플레이어가 다음에 진입할 노드(전투, 이벤트, 다음 층 등)를 선택하는 화면입니다.

---

## 🎯 주요 기능

1. **현재 층 정보 표시** - 몇 층인지, 설명 표시
2. **진행 통계 표시** - 현재 런에서의 전투 횟수, 처치 몬스터 수
3. **선택 가능한 노드 목록** - StageManager에서 가져온 노드들
4. **키보드 입력 처리** - ↑/↓로 선택, Enter로 진입
5. **Scene 전환** - 선택한 노드에 따라 다른 Scene으로 이동

---

## 🔄 StageManager 사용 흐름

### 1. Enter() - 노드 목록 가져오기

```cpp
void StageSelectScene::Enter()
{
  // UI 초기화
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
_Drawer->Activate();
    _IsActive = true;
    _SelectedNodeIndex = 0;

    // ===== StageManager에서 데이터 가져오기 =====
  StageManager* stageMgr = StageManager::GetInstance();
    
    // 1. 현재 선택 가능한 노드 가져오기
    auto availableNodes = stageMgr->GetAvailableNextNodes();
    
    // 2. UI 표시용 StageNode로 변환
    _CurrentNodes.clear();
    int index = 0;
 for (const auto* nodeData : availableNodes)
    {
        StageNode uiNode;
        uiNode.Type = nodeData->Type;
        uiNode.Index = index++;
        
 // 노드 이름 설정 (UI에 표시될 텍스트)
        switch (nodeData->Type)
    {
        case ENodeType::Battle:
  uiNode.Name = "전투 (" + nodeData->EnemyType + " x" + 
 std::to_string(nodeData->EnemyCount) + ")";
   break;
    case ENodeType::Elite:
            uiNode.Name = "엘리트 전투 (" + nodeData->EnemyType + " x" + 
         std::to_string(nodeData->EnemyCount) + ")";
     break;
        case ENodeType::Boss:
            uiNode.Name = "보스 전투";
 break;
     case ENodeType::Event:
       uiNode.Name = "이벤트: " + nodeData->EventType;
            break;
        case ENodeType::Exit:
            uiNode.Name = "다음 층으로";
  break;
        default:
            uiNode.Name = "알 수 없음";
            break;
        }
   
        _CurrentNodes.push_back(uiNode);
    }
    
    // 3. 현재 층 정보 가져오기
    const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();
    if (floorInfo)
    {
        _CurrentStageLevel = floorInfo->Floor;
    // 층 설명도 사용 가능: floorInfo->Description
    }
 
    // 4. 진행 통계 가져오기
    const StageProgress& progress = stageMgr->GetProgress();
    // progress.TotalBattlesCompleted
    // progress.NormalMonstersKilled
    // progress.EliteMonstersKilled
    // progress.BossesKilled
    // progress.EventsCleared
    
    // 패널 생성 및 렌더링...
    _Drawer->Render();
}
```

---

## 🎮 HandleInput() - 키 입력 및 Scene 전환

### 전체 구조

```cpp
void StageSelectScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;
    
    int keyCode = input->GetKeyCode();
    
    // 1. ↑/↓ 키: 노드 선택
    // 2. Enter: 노드 진입 (Scene 전환)
  // 3. ESC: 메인 메뉴 (게임 포기)
}
```

### 1. 노드 선택 (↑/↓)

```cpp
if (keyCode == VK_UP)  // ↑ 위로
{
    _SelectedNodeIndex--;
    if (_SelectedNodeIndex < 0)
        _SelectedNodeIndex = static_cast<int>(_CurrentNodes.size()) - 1;
    
    // 노드 패널 업데이트
    UpdateNodePanel();  // 패널 내용 다시 그리기
    _Drawer->Render();
}
else if (keyCode == VK_DOWN)  // ↓ 아래로
{
    _SelectedNodeIndex++;
    if (_SelectedNodeIndex >= static_cast<int>(_CurrentNodes.size()))
     _SelectedNodeIndex = 0;
    
    // 노드 패널 업데이트
    UpdateNodePanel();
  _Drawer->Render();
}
```

### 2. 노드 진입 및 Scene 전환 (Enter)

```cpp
else if (keyCode == VK_RETURN)  // Enter - 노드 진입
{
    if (_SelectedNodeIndex >= 0 && _SelectedNodeIndex < _CurrentNodes.size())
    {
        StageManager* stageMgr = StageManager::GetInstance();
     auto availableNodes = stageMgr->GetAvailableNextNodes();
 
        if (_SelectedNodeIndex < availableNodes.size())
      {
      const NodeData* selectedNode = availableNodes[_SelectedNodeIndex];
      
       // ===== StageManager에 이동 기록 =====
       if (stageMgr->MoveToNode(selectedNode->Id))
     {
        _IsActive = false;
       Exit();
       
    // ===== 노드 타입별 Scene 전환 =====
      SceneManager* sceneMgr = SceneManager::GetInstance();
        
       switch (selectedNode->Type)
      {
                case ENodeType::Battle:
      case ENodeType::Elite:
     // 일반/엘리트 전투 시작
         BattleManager::GetInstance()->StartBattle(EBattleType::Normal);
   sceneMgr->ChangeScene(ESceneType::Battle);
    break;
        
    case ENodeType::Boss:
  // 보스 전투 시작
    BattleManager::GetInstance()->StartBattle(EBattleType::Boss);
            sceneMgr->ChangeScene(ESceneType::Battle);
     break;
  
                case ENodeType::Event:
        // 이벤트 타입에 따라 분기
         if (selectedNode->EventType == "Companion")
        {
             sceneMgr->ChangeScene(ESceneType::CompanionRecruit);
             }
        // 다른 이벤트 타입 추가 가능
          break;
    
 case ENodeType::Exit:
         // 다음 층으로 이동
       if (stageMgr->MoveToNextFloor())
           {
            // 성공: 다시 StageSelect로 (새 층의 노드 표시)
          sceneMgr->ChangeScene(ESceneType::StageSelect);
 }
      else
    {
   // 실패 (10층 초과): 게임 클리어
        sceneMgr->ChangeScene(ESceneType::Result);
         }
   break;
           
        default:
       break;
  }
        }
}
    }
}
```

### 3. 메인 메뉴 복귀 (ESC)

```cpp
else if (keyCode == VK_ESCAPE)  // ESC - 메인 메뉴 (게임 포기)
{
    _IsActive = false;
    Exit();
    SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
}
```

---

## 🔄 다른 Scene에서 StageSelect로 복귀

### BattleScene::Exit() - 전투 종료 후

```cpp
void BattleScene::Exit()
{
    BattleManager* battleMgr = BattleManager::GetInstance();
StageManager* stageMgr = StageManager::GetInstance();
    
    // 전투 결과 확인
    const BattleResult& result = battleMgr->GetBattleResult();
    
    if (result.Victory && result.IsCompleted)
    {
        // 현재 노드 정보 가져오기
        const NodeData* currentNode = stageMgr->GetCurrentNode();
  
     if (currentNode)
 {
            // ===== 노드 완료 처리 (통계 업데이트) =====
            stageMgr->CompleteNode(currentNode->Type);
        }
    }
    
    // 패널 정리
    _Drawer->RemoveAllPanels();
    _IsActive = false;
    
    // ===== 다시 StageSelect로 복귀 =====
    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
}
```

### CompanionRecruitScene::Exit() - 동료 영입 후

```cpp
void CompanionRecruitScene::Exit()
{
    StageManager* stageMgr = StageManager::GetInstance();

    // 이벤트 완료 처리
    stageMgr->CompleteNode(ENodeType::Event);
 
    // 패널 정리
    _Drawer->RemoveAllPanels();
 _IsActive = false;
    
    // StageSelect로 복귀
    SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
}
```

---

## 📊 SceneManager 사용 패턴 정리

### 기본 사용법

```cpp
SceneManager* sceneMgr = SceneManager::GetInstance();

// Scene 전환
sceneMgr->ChangeScene(ESceneType::원하는씬);
```

### Scene 전환 시 주의사항

1. **Scene 전환 전 정리**
   ```cpp
   _IsActive = false;  // 현재 Scene 비활성화
   Exit();             // 패널 제거 등 정리
   sceneMgr->ChangeScene(ESceneType::다음씬);
   ```

2. **BattleManager와 함께 사용**
   ```cpp
   // 전투 시작 + Scene 전환
   BattleManager::GetInstance()->StartBattle(EBattleType::Normal);
   SceneManager::GetInstance()->ChangeScene(ESceneType::Battle);
   ```

3. **StageManager 상태 업데이트**
   ```cpp
   // 노드 이동 후 Scene 전환
   if (stageMgr->MoveToNode(nodeId))
   {
   sceneMgr->ChangeScene(ESceneType::다음씬);
   }
   
   // 노드 완료 후 복귀
   stageMgr->CompleteNode(nodeType);
   sceneMgr->ChangeScene(ESceneType::StageSelect);
   ```

---

## 🎨 UI 패널 업데이트 예시

### 노드 목록 표시

```cpp
void StageSelectScene::UpdateNodePanel()
{
    Panel* nodePanel = _Drawer->GetPanel("Nodes");
    auto nodeText = std::make_unique<TextRenderer>();
    
  nodeText->AddLine("");
  nodeText->AddLine("  선택 가능한 경로:");
    nodeText->AddLine("");
    
for (size_t i = 0; i < _CurrentNodes.size(); ++i)
  {
        std::string nodeIcon = "";
        ETextColor nodeColor = ETextColor::WHITE;
        
        // 노드 타입별 아이콘 및 색상
      switch (_CurrentNodes[i].Type)
      {
        case ENodeType::Battle:
            nodeIcon = "[전투]";
nodeColor = ETextColor::LIGHT_RED;
    break;
        case ENodeType::Elite:
  nodeIcon = "[엘리트]";
     nodeColor = ETextColor::LIGHT_MAGENTA;
  break;
        case ENodeType::Boss:
      nodeIcon = "[보스]";
   nodeColor = ETextColor::YELLOW;
    break;
        case ENodeType::Event:
      nodeIcon = "[이벤트]";
      nodeColor = ETextColor::LIGHT_GREEN;
  break;
        case ENodeType::Exit:
   nodeIcon = "[다음층]";
            nodeColor = ETextColor::LIGHT_CYAN;
            break;
        }
        
        // 선택된 노드는 노란색으로 강조
     std::string prefix = (i == _SelectedNodeIndex) ? "> " : "";
      std::string nodeLine = prefix + nodeIcon + " " + _CurrentNodes[i].Name;
     
        if (i == _SelectedNodeIndex)
        {
      nodeText->AddLineWithColor(nodeLine,
                MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        }
  else
     {
    nodeText->AddLineWithColor(nodeLine,
      MakeColorAttribute(nodeColor, EBackgroundColor::BLACK));
    }
        
        nodeText->AddLine("");
    }
    
    nodePanel->SetContentRenderer(std::move(nodeText));
    nodePanel->Redraw();
}
```

---

## 🔍 디버깅 팁

### 로그 출력으로 상태 확인

```cpp
#include "../../include/Manager/PrintManager.h"

void StageSelectScene::Enter()
{
    StageManager* stageMgr = StageManager::GetInstance();
    const StageProgress& progress = stageMgr->GetProgress();
    
    // 디버그 로그 출력
    PrintManager* pm = PrintManager::GetInstance();
    pm->PrintLogLine("===== StageSelect Enter =====", ELogImportance::DISPLAY);
    pm->PrintLogLine("Current Floor: " + std::to_string(progress.CurrentFloor), ELogImportance::DISPLAY);
    pm->PrintLogLine("Current Node: " + progress.CurrentNodeId, ELogImportance::DISPLAY);
    pm->PrintLogLine("Battles: " + std::to_string(progress.TotalBattlesCompleted), ELogImportance::DISPLAY);
}
```

### 노드 연결 확인

```cpp
auto availableNodes = stageMgr->GetAvailableNextNodes();
PrintManager* pm = PrintManager::GetInstance();

pm->PrintLogLine("Available Nodes: " + std::to_string(availableNodes.size()), ELogImportance::DISPLAY);
for (const auto* node : availableNodes)
{
    pm->PrintLogLine(" - " + node->Id + " (" + std::to_string((int)node->Type) + ")", ELogImportance::DISPLAY);
}
```

---

## ✅ 체크리스트

구현 시 확인할 사항:

- [ ] Enter()에서 StageManager로 노드 목록 가져오기
- [ ] _CurrentNodes에 UI용 데이터 변환
- [ ] 현재 층 정보 표시
- [ ] 진행 통계 표시
- [ ] ↑/↓ 키로 노드 선택
- [ ] Enter 키로 노드 진입
- [ ] MoveToNode() 호출 후 Scene 전환
- [ ] 노드 타입별 올바른 Scene으로 이동
- [ ] Exit 노드에서 MoveToNextFloor() 처리
- [ ] 10층 초과 시 Result Scene으로 이동
- [ ] ESC 키로 메인 메뉴 복귀

---

## 🎯 요약

1. **Enter()**: StageManager에서 노드 목록 가져와 UI에 표시
2. **HandleInput()**: 키 입력 처리 및 SceneManager로 Scene 전환
3. **Exit()**: 패널 정리 및 Scene 비활성화
4. **다른 Scene**: 작업 완료 후 CompleteNode() 호출 → StageSelect로 복귀

StageSelectScene은 StageManager와 SceneManager의 중개자 역할을 합니다! 🚀
