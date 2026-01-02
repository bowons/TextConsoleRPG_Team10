#include <Windows.h>
#include "include/Manager/GameManager.h"

void main()
{
    SetConsoleOutputCP(65001);
    GameManager::GetInstance()->StartGame();
}