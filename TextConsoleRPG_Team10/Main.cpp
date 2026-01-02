#include "include/Manager/GameManager.h"
#include "include/Manager/DataManager.h"
#include "include/Manager/PrintManager.h"
#include "SoundPlayer.h"
#include <Windows.h>

int main()
{
    // 1. 콘솔 인코딩 설정 (한글 깨짐 방지)
    SetConsoleOutputCP(65001);

    // 2. 매니저들 초기화 (DataManager -> SoundPlayer 순서 권장)
    if (!DataManager::GetInstance()->Initialize()) {
        PrintManager::GetInstance()->PrintLogLine("DataManager 초기화 실패!", ELogImportance::WARNING);
        return -1;
    }

    if (!SoundPlayer::GetInstance()->Initialize()) {
        PrintManager::GetInstance()->PrintLogLine("SoundPlayer 초기화 실패!", ELogImportance::WARNING);
        return -1;
    }

    PrintManager::GetInstance()->PrintLogLine("시스템 초기화 완료. 테스트를 시작합니다.",ELogImportance::DISPLAY);
    PrintManager::GetInstance()->PrintLogLine("[1] 배경음악 재생 (Loop)");
    PrintManager::GetInstance()->PrintLogLine("[2] 효과음 재생 (Play)");
    PrintManager::GetInstance()->PrintLogLine("[ESC] 종료");

    // 테스트용 배경음악 재생 (파일이 해당 경로에 있어야 합니다)
    // 예: Source/Game/Assets/Audio/BGM_Main.mp3
    SoundPlayer::GetInstance()->PlayLoop(
        "812426__dinisnakamura__battle_bgm_castle_dragon_extended.wav",
        "MainBGM", 0.3f);

    bool bRunning = true;
    while (bRunning) {
        // 키 입력 처리 (비동기 방식 테스트)
        if (GetAsyncKeyState('2') & 0x8000) {
            // 효과음 재생 (짧은 사운드 파일)
            SoundPlayer::GetInstance()->Play("swish_2.wav", 0.8f);
            Sleep(100); // 연속 입력 방지
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            bRunning = false;
        }

        // 3. ★가장 중요★ 매 프레임 사운드 상태 업데이트
        // 이 함수가 호출되어야 재생 끝난 사운드가 메모리에서 해제됩니다.
        SoundPlayer::GetInstance()->Update();

        // CPU 점유율 과다 방지를 위한 미세한 대기
        Sleep(10);
    }

    // 4. 종료 시 모든 사운드 정지 및 리소스 해제
    SoundPlayer::GetInstance()->StopAll();

    PrintManager::GetInstance()->PrintLogLine("테스트를 종료합니다.");
    return 0;
}
