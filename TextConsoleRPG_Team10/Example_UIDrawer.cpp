/*
 * UIDrawer 사용 예제
 * 
 * 이 파일은 UIDrawer 시스템의 사용법을 보여주는 예제입니다.
 * 전투 화면을 구성하는 방법을 단계별로 설명합니다.
 * 
 * 주의: 이 파일은 예제이므로 직접 컴파일되지 않습니다.
 *    실제 사용 시 GameManager나 BattleManager에 통합하세요.
 */

#include "include/UI/UIDrawer.h"
#include "include/UI/Panel.h"
#include "include/UI/TextRenderer.h"
#include "include/UI/StatRenderer.h"
#include "include/UI/AsciiArtRenderer.h"
#include "include/Manager/DataManager.h"
#include "include/Manager/PrintManager.h"
#include "include/Unit/Player.h"
#include "include/Unit/IMonster.h"

// ===== 예제 1: 기본 UI 설정 =====
void Example_BasicUI()
{
    // 1. UIDrawer 초기화
    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->Initialize(106, 65);  // 콘솔 크기
    drawer->Activate();           // UI 모드 활성화

    // 2. 패널 생성 (ID, X, Y, Width, Height)
    Panel* titlePanel = drawer->CreatePanel("Title", 0, 0, 106, 5);
    titlePanel->SetBorder(true, 14);  // 테두리: 노란색

  // 3. 콘텐츠 렌더러 설정
  auto textRenderer = std::make_unique<TextRenderer>();
    textRenderer->AddLine("=== 전투 시작 ===");
    textRenderer->SetTextColor(14);  // 노란색
    titlePanel->SetContentRenderer(std::move(textRenderer));

    // 4. 화면에 출력
    drawer->Render();

    // 5. 종료
    drawer->Deactivate();
}

// ===== 예제 2: 전투 화면 구성 =====
void Example_BattleScreen(Player* player, IMonster* monster)
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->Initialize();
    drawer->Activate();

    // === 상단: 전투 정보 ===
    Panel* headerPanel = drawer->CreatePanel("Header", 0, 0, 106, 3);
    headerPanel->SetBorder(true, 11);
    auto headerText = std::make_unique<TextRenderer>();
    headerText->AddLine("[전투] " + monster->GetStage());
    headerText->SetTextColor(11);
    headerPanel->SetContentRenderer(std::move(headerText));

    // === 좌측: 플레이어 스탯 ===
    Panel* playerPanel = drawer->CreatePanel("PlayerStats", 0, 3, 30, 15);
    playerPanel->SetBorder(true, 10);
    auto playerStats = std::make_unique<StatRenderer>();
    playerStats->SetStat("이름", player->GetName());
    playerStats->SetStat("레벨", player->GetLevel());
    playerStats->SetStat("HP", std::to_string(player->GetCurrentHP()) + "/" + std::to_string(player->GetMaxHP()));
    playerStats->SetStat("ATK", player->GetAtk());
    playerPanel->SetContentRenderer(std::move(playerStats));

    // === 중앙: 몬스터 아스키 아트 ===
    Panel* monsterArtPanel = drawer->CreatePanel("MonsterArt", 30, 3, 46, 30);
    monsterArtPanel->SetBorder(true, 12);
    auto monsterArt = std::make_unique<AsciiArtRenderer>();
    
    // DataManager를 통해 아스키 아트 로드
    std::string monstersPath = DataManager::GetInstance()->GetResourcePath("Monsters");
    monsterArt->LoadFromFile(monstersPath, monster->GetName() + ".txt");
    monsterArt->SetAlignment(ArtAlignment::CENTER);
    monsterArt->SetColor(12);  // 빨간색
    monsterArtPanel->SetContentRenderer(std::move(monsterArt));

    // === 우측: 몬스터 스탯 ===
    Panel* monsterPanel = drawer->CreatePanel("MonsterStats", 76, 3, 30, 15);
    monsterPanel->SetBorder(true, 12);
    auto monsterStats = std::make_unique<StatRenderer>();
    monsterStats->SetStat("이름", monster->GetName());
    monsterStats->SetStat("레벨", monster->GetLevel());
    monsterStats->SetStat("HP", std::to_string(monster->GetCurrentHP()) + "/" + std::to_string(monster->GetMaxHP()));
    monsterStats->SetStat("ATK", monster->GetAtk());
    monsterPanel->SetContentRenderer(std::move(monsterStats));

    // === 하단: 전투 로그 ===
    Panel* logPanel = drawer->CreatePanel("BattleLog", 0, 40, 106, 25);
    logPanel->SetBorder(true, 7);
    auto logRenderer = std::make_unique<TextRenderer>();
    logRenderer->AddLine("전투가 시작되었습니다!");
    logRenderer->SetTextColor(7);
    logPanel->SetContentRenderer(std::move(logRenderer));

    // 첫 렌더링
  drawer->Render();

    // === 전투 루프 (예시) ===
    while (!player->IsDead() && !monster->IsDead())
    {
        // 플레이어 공격
        player->Attack(monster);
        
  // 로그 업데이트
        Panel* log = drawer->GetPanel("BattleLog");
        TextRenderer* logContent = dynamic_cast<TextRenderer*>(log->GetContentRenderer());
if (logContent) {
            logContent->AddLine(player->GetName() + "의 공격! " + std::to_string(player->GetAtk()) + " 데미지!");
     }

      // 몬스터 스탯 업데이트
    Panel* monsterStatsPanel = drawer->GetPanel("MonsterStats");
        StatRenderer* monsterStatsContent = dynamic_cast<StatRenderer*>(monsterStatsPanel->GetContentRenderer());
        if (monsterStatsContent) {
monsterStatsContent->SetStat("HP", std::to_string(monster->GetCurrentHP()) + "/" + std::to_string(monster->GetMaxHP()));
        }

   // 화면 갱신
        drawer->Update();  // 애니메이션 업데이트 + 렌더링

        Sleep(1000);  // 1초 대기

        if (monster->IsDead()) break;

        // 몬스터 공격
      monster->Attack(player);
        
        if (logContent) {
            logContent->AddLine(monster->GetName() + "의 공격! " + std::to_string(monster->GetAtk()) + " 데미지!");
        }

      // 플레이어 스탯 업데이트
        Panel* playerStatsPanel = drawer->GetPanel("PlayerStats");
        StatRenderer* playerStatsContent = dynamic_cast<StatRenderer*>(playerStatsPanel->GetContentRenderer());
        if (playerStatsContent) {
    playerStatsContent->SetStat("HP", std::to_string(player->GetCurrentHP()) + "/" + std::to_string(player->GetMaxHP()));
        }

    drawer->Update();
   Sleep(1000);
    }

    // 종료
    drawer->Deactivate();
}

// ===== 예제 3: 애니메이션 =====
void Example_Animation()
{
    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->Initialize();
    drawer->Activate();

    Panel* animPanel = drawer->CreatePanel("Animation", 20, 10, 60, 40);
    animPanel->SetBorder(true);

    auto artRenderer = std::make_unique<AsciiArtRenderer>();
    
    // 여러 프레임 로드
 std::vector<std::string> frameFiles = {
        "Monster_Frame1.txt",
      "Monster_Frame2.txt",
        "Monster_Frame3.txt"
  };
    
    std::string monstersPath = DataManager::GetInstance()->GetResourcePath("Monsters");
    artRenderer->LoadAnimationFromFiles(monstersPath, frameFiles, 0.3f);  // 0.3초마다 프레임 전환
    artRenderer->StartAnimation();
    
    animPanel->SetContentRenderer(std::move(artRenderer));

    // 10초간 애니메이션 재생
    for (int i = 0; i < 300; ++i) {  // 30 FPS * 10초
        drawer->Update();
    }

    drawer->Deactivate();
}

// ===== 예제 4: 레거시 코드와 병행 사용 =====
void Example_LegacyCompatibility()
{
    // 레거시 방식 (기존 코드)
    PrintManager::GetInstance()->PrintLogLine("이것은 기존 방식입니다.");
    
    // --- 여기서 UI Drawer 모드로 전환 ---
    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->Initialize();
    drawer->Activate();

    // UI Drawer 방식
    Panel* panel = drawer->CreatePanel("Message", 10, 10, 80, 10);
    panel->SetBorder(true);
 
    auto text = std::make_unique<TextRenderer>();
    text->AddLine("이것은 새로운 UI Drawer 방식입니다.");
    panel->SetContentRenderer(std::move(text));
    
    drawer->Render();
    Sleep(2000);

    // --- 다시 레거시 모드로 복귀 ---
    drawer->Deactivate();
    
    // 레거시 방식 재개
    PrintManager::GetInstance()->PrintLogLine("다시 기존 방식으로 돌아왔습니다.");
}

// ===== 예제 5: 동적 업데이트 =====
void Example_DynamicUpdate(Player* player)
{
 UIDrawer* drawer = UIDrawer::GetInstance();
 drawer->Initialize();
    drawer->Activate();

    Panel* statsPanel = drawer->CreatePanel("DynamicStats", 30, 20, 40, 10);
    statsPanel->SetBorder(true);
    
    auto stats = std::make_unique<StatRenderer>();
    stats->SetStat("HP", std::to_string(player->GetCurrentHP()));
    stats->SetStat("Gold", player->GetGold());
    statsPanel->SetContentRenderer(std::move(stats));

    drawer->Render();

 // 5초마다 스탯 변경
    for (int i = 0; i < 5; ++i) {
        Sleep(5000);

        // 플레이어 HP 감소 시뮬레이션
        player->TakeDamage(10);

// 스탯 업데이트
        StatRenderer* statsContent = dynamic_cast<StatRenderer*>(statsPanel->GetContentRenderer());
    if (statsContent) {
  statsContent->SetStat("HP", std::to_string(player->GetCurrentHP()));
        }

        // 강제 재렌더링
        statsPanel->Redraw();
        drawer->Render();
    }

    drawer->Deactivate();
}
