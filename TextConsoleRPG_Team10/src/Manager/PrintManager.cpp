#include "../../include/Manager/PrintManager.h"
#include "Windows.h"
#include <iostream>

using namespace std;

void PrintManager::PrintLog(const string& Msg, ELogImportance Importance)
{
    ETextColor PrevColor = GetCurrentTextColor();
    switch (Importance)
    {
    case ELogImportance::DISPLAY:
    {
        ChangeTextColor(ETextColor::YELLOW);
        cout << "[DISPLAY]: ";
    }
        break;
    case ELogImportance::WARNING:
    {
        ChangeTextColor(ETextColor::RED);
        cout << "[WARNING]: ";
    }
        break;
    default:
        break;
    }

    for (int i = 0; i < Msg.length(); i += _LineLimit)
    {
        string MsgStr = Msg.substr(i, _LineLimit);
        cout << MsgStr << '\n';
    }
    ChangeTextColor(PrevColor);
}

void PrintManager::PrintWithTyping(const string& Msg)
{
    int _Interval = 0;
    switch (_CurrentSpeed)
    {
    case ETypingSpeed::Slow:
    {
        _Interval = 400;
    }
        break;
    case ETypingSpeed::Normal:
    {
        _Interval = 200;
    }

        break;
    case ETypingSpeed::Fast:
    {
        _Interval = 100;
    }
        break;
    default:
        break;
    }

    for (int i = 0; i < Msg.length(); i++)
    {
        cout << Msg[i];
        Sleep(Msg[i] == ' ' ? _Interval : _Interval * 2);
        // 줄당 글자 제한수에 걸리면 다음 줄로 개행
        if (i != 0 && i % _LineLimit == 0)
        {
            cout << '\n';
        }
    }
    cout << '\n';
}

void PrintManager::ChangeTextColor(ETextColor NewTextColor)
{
    int ColorIndex = static_cast<int>(NewTextColor);
    if (NewTextColor >= ETextColor::MAX)
    {
        cout << "Invalid Text Color Index\n";
        return;
    }

    HANDLE HConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(HConsole, ColorIndex);
}

ETextColor PrintManager::GetCurrentTextColor()
{
    HANDLE HConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    
    if (GetConsoleScreenBufferInfo(HConsole, &ScreenBufferInfo))
    {
        // wAttributes - 8 byte, 상위 4byte는 배경, 하위 4byte는 글자 색상 
        return static_cast<ETextColor>(ScreenBufferInfo.wAttributes & 0x0F);
    }

    // 실패 시 기본값
    return ETextColor::WHITE;
}

void PrintManager::SetLineLimit(int Limit)
{
    if (Limit <= 0)Limit = 1;
    _LineLimit = Limit;
}

int PrintManager::GetLineLimit()
{
    return _LineLimit;
}

void PrintManager::SetTypingSpeed(ETypingSpeed NewSpeed)
{
    if (NewSpeed > ETypingSpeed::MAX || NewSpeed <= ETypingSpeed::NONE) NewSpeed = ETypingSpeed::Slow;
    _CurrentSpeed = NewSpeed;
}
