#include "include/Manager/GameManager.h"
#include <Windows.h>

int main()
{
    SetConsoleOutputCP(65001);

    GameManager::GetInstance()->StartGame();
    return 0;
}