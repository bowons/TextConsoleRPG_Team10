#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/SceneManager.h"
#include "../../include/Manager/StageManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/UI/UIDrawer.h"
#include "../../include/UI/Scenes/MainMenuScene.h"
#include "../../include/UI/Scenes/PlayerNameInputScene.h"
#include "../../include/UI/Scenes/CharacterSelectScene.h"
#include "../../include/UI/Scenes/StoryProgressScene.h"
#include "../../include/UI/Scenes/StageSelectScene.h"
#include "../../include/UI/Scenes/BattleScene.h"
#include "../../include/UI/Scenes/ShopScene.h"
#include "../../include/UI/Scenes/CompanionRecruitScene.h"
#include "../../include/UI/Scenes/ResultScene.h"
#include "../../include/Unit/Player.h"
#include <Windows.h>
#include <algorithm>

// ===== 파티 관리 구현 (Player 정의 필요) =====
void GameManager::RemoveDeadCompanions()
{
    // 메인 플레이어(0번)는 제외하고 동료들만 검사 - 메인 플레이어 죽으면 게임 끝남!
    if (_Party.size() <= 1) return;  // 메인 플레이어만 있으면 스킵
    
    auto it = _Party.begin() + 1;  // 1번 인덱스부터 시작 (0번은 메인 플레이어)
    while (it != _Party.end())
    {
        if (*it && (*it)->IsDead())
        {
            it = _Party.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

size_t GameManager::GetAliveCount() const
{
    return std::count_if(_Party.begin(), _Party.end(),
        [](const std::shared_ptr<Player>& member) {
            return member && !member->IsDead();
        });
}

// ===== 게임 초기화 (씬 등록) =====
void GameManager::Initialize()
{
    PrintManager::GetInstance()->PrintLogLine(
        "GameManager: 게임 초기화 중...",
        ELogImportance::DISPLAY
    );

    // DataManager 초기화
    if (!DataManager::GetInstance()->Initialize())
    {
        PrintManager::GetInstance()->PrintLogLine(
         "DataManager 초기화 실패!",
         ELogImportance::WARNING
        );
    return;
    }

    // StageManager 초기화
    if (!StageManager::GetInstance()->Initialize())
    {
        PrintManager::GetInstance()->PrintLogLine(
    "StageManager 초기화 실패!",
      ELogImportance::WARNING
   );
        return;
    }

    // UIDrawer 초기화
    UIDrawer* drawer = UIDrawer::GetInstance();
    if (!drawer->Initialize(150, 45))
    {
        PrintManager::GetInstance()->PrintLogLine(
            "UIDrawer 초기화 실패!",
            ELogImportance::WARNING
        );
        return;
    }

    // SceneManager 가져오기
    SceneManager* sm = SceneManager::GetInstance();

    // ===== 씬 등록 (확장 가능 구조) =====
    PrintManager::GetInstance()->PrintLogLine(
        "SceneManager: 게임 화면 로딩 중...",
        ELogImportance::DISPLAY
    );

    // 완전 구현된 씬
    sm->RegisterScene(ESceneType::MainMenu, std::make_unique<MainMenuScene>());
    sm->RegisterScene(ESceneType::PlayerNameInput, std::make_unique<PlayerNameInputScene>());

    // 빈 껍데기 씬 (팀원이 구현 예정)
    sm->RegisterScene(ESceneType::CharacterSelect, std::make_unique<CharacterSelectScene>());
    sm->RegisterScene(ESceneType::StoryProgress, std::make_unique<StoryProgressScene>());
    sm->RegisterScene(ESceneType::StageSelect, std::make_unique<StageSelectScene>());
    sm->RegisterScene(ESceneType::Battle, std::make_unique<BattleScene>());
    sm->RegisterScene(ESceneType::Shop, std::make_unique<ShopScene>());
    sm->RegisterScene(ESceneType::CompanionRecruit, std::make_unique<CompanionRecruitScene>());
    sm->RegisterScene(ESceneType::Result, std::make_unique<ResultScene>());

    PrintManager::GetInstance()->PrintLogLine(
        "GameManager: 초기화 완료!",
        ELogImportance::DISPLAY
    );
}

// ===== 게임 시작 (메인 루프) =====
void GameManager::StartGame()
{
    _IsRunning = true;
    _IsGameOver = false;

    PrintManager::GetInstance()->PrintLogLine(
        "==========에레보스 타워: 뒤틀린 성의 종언==========",
        ELogImportance::DISPLAY
    );
    PrintManager::GetInstance()->EndLine();

    SceneManager* sm = SceneManager::GetInstance();
    UIDrawer* drawer = UIDrawer::GetInstance();

    // 메인 메뉴부터 시작
    sm->ChangeScene(ESceneType::MainMenu);

    // ===== 메인 게임 루프 =====
    while (_IsRunning && !_IsGameOver)
    {
        // 씬 업데이트
        sm->Update();

        // 씬 렌더링
        sm->Render();

        // 종료 조건 확인
        UIScene* currentScene = sm->GetCurrentScene();
        if (currentScene && !currentScene->IsActive())
        {
            // MainMenu에서 종료 선택 시
            if (sm->GetCurrentSceneType() == ESceneType::MainMenu)
            {
                EndGame();
            }
        }
    }

    // 게임 종료 메시지
    PrintManager::GetInstance()->EndLine();
    PrintManager::GetInstance()->PrintLogLine(
        "게임을 종료합니다. 플레이해 주셔서 감사합니다!",
        ELogImportance::DISPLAY
    );

    // UIDrawer 정리
    drawer->Shutdown();
}

// ===== 게임 종료 =====
void GameManager::EndGame()
{
    _IsGameOver = true;
    _IsRunning = false;
}

// ===== 배틀 테스트 (임시) =====
void GameManager::StartBattleTest()
{
  _IsGameOver = false;
  _IsRunning = true;

  PrintManager::GetInstance()->PrintLogLine(
	  "배틀 테스트 모드 시작...",
	  ELogImportance::DISPLAY);
  PrintManager::GetInstance()->EndLine();

  // 임시 파티 생성 (메인 플레이어 + 용병 3명)
  auto mainPlayer = std::make_shared<Player>("플레이어", true);
  auto companion1 = std::make_shared<Player>("용병 A", false);
  auto companion2 = std::make_shared<Player>("용병 B", false);
  auto companion3 = std::make_shared<Player>("용병 C", false);

  _Party.clear();
  _Party.push_back(mainPlayer);
  _Party.push_back(companion1);
  _Party.push_back(companion2);
  _Party.push_back(companion3);

  SceneManager* sm = SceneManager::GetInstance();
  UIDrawer* drawer = UIDrawer::GetInstance();

  // 배틀 씬으로 바로 전환
  sm->ChangeScene(ESceneType::Battle);

  // ===== 메인 게임 루프 =====
  while (_IsRunning && !_IsGameOver) {
    // 씬 업데이트
    sm->Update();

    // 씬 렌더링
    sm->Render();

    // ESC 키로 종료 가능 (테스트용)
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
      Sleep(150);
      EndGame();
    }
  }

  // 게임 종료 메시지
  PrintManager::GetInstance()->EndLine();
  PrintManager::GetInstance()->PrintLogLine("전투 테스트를 종료합니다.",
                                            ELogImportance::DISPLAY);

  // UIDrawer 정리
  drawer->Shutdown();
}