#include "include/Manager/GameManager.h"
#include "include/Manager/DataManager.h"
#include "include/Manager/PrintManager.h"
#include <Windows.h>
#include <iostream>

// UIDrawer 테스트 함수 선언
void RunUIDrawerTest();

// 콘솔 창 크기 고정 함수
void SetConsoleWindowSize(int width, int height)
{
    char command[256];
    sprintf_s(command, "mode con: cols=%d lines=%d", width, height);
    system(command);

    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow)
    {
        LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
        style &= ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
        SetWindowLong(consoleWindow, GWL_STYLE, style);
        SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

int main()
{
    // 1. 콘솔 인코딩 설정
    SetConsoleOutputCP(65001);

    //RunUIDrawerTest();

    // 2. 콘솔 창 크기 고정
    SetConsoleWindowSize(150, 45);

    // 3. DataManager 초기화
    if (!DataManager::GetInstance()->Initialize()) {
        PrintManager::GetInstance()->PrintLogLine(
            "DataManager 초기화 실패!",
            ELogImportance::WARNING
        );
        return -1;
    }

    // ========================================
    // 게임 시작 (Scene 시스템)
    // ========================================
    GameManager* gm = GameManager::GetInstance();

    // 게임 초기화 (씬 등록)
    gm->Initialize();

    // 게임 시작 (메인 루프)
    gm->StartCompanionRecruitTest();

    return 0;
}
