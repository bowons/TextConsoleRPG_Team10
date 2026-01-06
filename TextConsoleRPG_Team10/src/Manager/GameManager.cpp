#include "../../include/Manager/GameManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/SceneManager.h"
#include "../../include/Manager/StageManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/SoundPlayer.h"
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
#include "../../include/Unit/Warrior.h"
#include "../../include/Unit/Mage.h"
#include "../../include/Unit/Archer.h"
#include "../../include/Unit/Priest.h"
#include "../../include/Data/ClassData.h"  // 추가
#include <Windows.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>

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
    // 랜덤 시드 초기화 (치명타 판정용)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

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

    // 로딩 느낌 내는 용도, 자유롭게 제거
    // SoundPlayer::GetInstance()->PlaySFXWithPause("Title_Enter");
    SoundPlayer::GetInstance()->PlayBGM("BGM_NoneSet01");

    // ===== 메인 게임 루프 =====
    while (_IsRunning && !_IsGameOver)
    {
        // 씬 업데이트
        sm->Update();

        // 씬 렌더링
        sm->Render();

        // 사운드 해제
        SoundPlayer::GetInstance()->Update();

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

// ===== 게임 재시작 =====
void GameManager::RestartGame()
{
    PrintManager::GetInstance()->PrintLogLine(
        "게임을 재시작합니다...",
        ELogImportance::DISPLAY
    );

    ClearParty();

    StageManager* stageMgr = StageManager::GetInstance();
    if (stageMgr && stageMgr->IsInitialized())
    {
        stageMgr->StartNewGame();
    }

    _IsGameOver = false;
    _IsRunning = true;

    UIDrawer* drawer = UIDrawer::GetInstance();
    drawer->ClearScreen();
    drawer->RemoveAllPanels();

    PrintManager::GetInstance()->PrintLogLine(
        "새로운 여정을 시작합니다!",
        ELogImportance::DISPLAY
    );

    SceneManager::GetInstance()->ChangeScene(ESceneType::MainMenu);
}

// ===== DEPRECATED: 레거시 함수, CreateMainPlayerWithClass 사용 권장 =====
void GameManager::CreateMainPlayer(const std::string& name)
{
    // CSV 기반 생성자 사용 권장
    // TODO: 이 함수는 삭제 예정
    _MainPlayer = std::make_shared<Warrior>(name, true);
    _Party.clear();
    _Party.push_back(_MainPlayer);
}

bool GameManager::CreateMainPlayerWithClass(const std::string& name, const std::string& classId)
{
    DataManager* dm = DataManager::GetInstance();

    // Class.csv에서 직업 데이터 로드
    std::optional<ClassData> classDataOpt = dm->GetClassData(classId);

    if (!classDataOpt.has_value())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "직업 데이터를 불러올 수 없습니다: " + classId,
            ELogImportance::WARNING
        );
        return false;
    }

    const ClassData& data = classDataOpt.value();

    // 직업별 플레이어 생성 (CSV 데이터 사용)
    std::shared_ptr<Player> newPlayer;

    if (classId == "warrior")
    {
        newPlayer = std::make_shared<Warrior>(data, name, true);
    }
    else if (classId == "mage")
    {
        newPlayer = std::make_shared<Mage>(data, name, true);
    }
    else if (classId == "archer")
    {
        newPlayer = std::make_shared<Archer>(data, name, true);
    }
    else if (classId == "priest")
    {
        newPlayer = std::make_shared<Priest>(data, name, true);
    }
    else
    {
        PrintManager::GetInstance()->PrintLogLine(
            "알 수 없는 직업 ID: " + classId,
            ELogImportance::WARNING
        );
        return false;
    }

    // 파티 설정
    _MainPlayer = newPlayer;
    _Party.clear();
    _Party.push_back(_MainPlayer);

    PrintManager::GetInstance()->PrintLogLine(
        name + " (" + data._Role + ") 생성 완료!",
        ELogImportance::DISPLAY
    );

    return true;
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

    // 임시 파티 생성 (직업별로 테스트)
    auto mainPlayer = std::make_shared<Warrior>("전사", true);
    auto companion1 = std::make_shared<Mage>("마법사", false);
    auto companion2 = std::make_shared<Archer>("궁수", false);
    auto companion3 = std::make_shared<Priest>("사제", false);

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

// ===== 동료 영입 테스트 (임시)=====
void GameManager::StartCompanionRecruitTest() {
  _IsGameOver = false;
  _IsRunning = true;

  PrintManager::GetInstance()->PrintLogLine("동료 영입 테스트 모드 시작...",
                                            ELogImportance::DISPLAY);
  PrintManager::GetInstance()->EndLine();

  // 임시 메인 플레이어 생성
  auto mainPlayer = std::make_shared<Warrior>("테스트 플레이어", true);

  _Party.clear();
  _Party.push_back(mainPlayer);

  SceneManager* sm = SceneManager::GetInstance();
  UIDrawer* drawer = UIDrawer::GetInstance();

  // CompanionRecruitScene으로 바로 전환
  sm->ChangeScene(ESceneType::CompanionRecruit);

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
  PrintManager::GetInstance()->PrintLogLine("동료 영입 테스트를 종료합니다.",
                                            ELogImportance::DISPLAY);

  // UIDrawer 정리
  drawer->Shutdown();
}