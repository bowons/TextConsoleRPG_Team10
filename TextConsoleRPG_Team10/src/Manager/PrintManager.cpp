#include "../../include/Manager/PrintManager.h"
#include "Windows.h"
#include <iostream>

using namespace std;

// 로그 및 일반 출력 함수 - 개행 없음
void PrintManager::PrintLog(const string& Msg, ELogImportance Importance)
{
    ETextColor PrevColor = GetCurrentTextColor();
    switch (Importance)
    {
    // 일반 강조
    case ELogImportance::DISPLAY:
    {
        ChangeTextColor(ETextColor::YELLOW);
        cout << "[DISPLAY]: ";
    }
    break;
    // 심각함 강조 - 에러나 위험 표시에 사용
    case ELogImportance::WARNING:
    {
        ChangeTextColor(ETextColor::RED);
        cout << "[WARNING]: ";
    }
    break;
    default:
        break;
    }

    int LineCnt = Msg.length() / _LineLimit + 1;
    for (int i = 0; i < LineCnt; ++i)
    {
        string MsgStr = Msg.substr(i* _LineLimit, _LineLimit);
        cout << MsgStr;
        if (i != LineCnt - 1)
        {
            EndLine();
        }
    }

    ChangeTextColor(PrevColor);
}

// 로그 및 일반 출력 함수 - 개행 있음
void PrintManager::PrintLogLine(const string& Msg, ELogImportance Importance)
{
    PrintLog(Msg, Importance);
    EndLine();
}

// 타이핑 효과 적용 출력 함수 - 개행 없음
void PrintManager::PrintWithTyping(const string& Msg)
{
    int _Interval = 0;
    switch (_CurrentSpeed)
    {
    case ETypingSpeed::Slow:
    {
        _Interval = 200;
    }
    break;
    case ETypingSpeed::Normal:
    {
        _Interval = 100;
    }

    break;
    case ETypingSpeed::Fast:
    {
        _Interval = 50;
    }
    break;
    default:
        break;
    }

    for (const char& ch:Msg)
    {
        cout << ch;
        Sleep(ch == ' ' ? _Interval : _Interval * 2);
        // 줄당 글자 제한수에 걸리면 다음 줄로 개행
        ++_CurrentCharCnt;
        if (_CurrentCharCnt >= _LineLimit)
        {
            EndLine();
        }
    }
}

// 타이핑 효과 적용 출력 함수 - 개행 있음
void PrintManager::PrintWithTypingLine(const string& Msg)
{
    PrintWithTyping(Msg);
    EndLine();
}

// 텍스트 색상 변경
void PrintManager::ChangeTextColor(ETextColor NewTextColor)
{
    int ColorIndex = static_cast<int>(NewTextColor);
    if (NewTextColor >= ETextColor::MAX)
    {
        cout << "\nInvalid Text Color Index: "<< static_cast<int>(NewTextColor);
        EndLine();
        return;
    }

    HANDLE HConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(HConsole, ColorIndex);
}

// 현재 텍스트 색상 반환
const ETextColor PrintManager::GetCurrentTextColor()
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

// 개행
void PrintManager::EndLine()
{
    cout << '\n';
    _CurrentCharCnt = 0;
}

void PrintManager::SetLineLimit(int Limit)
{
    if (Limit <= 0)Limit = 1;
    _LineLimit = Limit;
}

const int& PrintManager::GetLineLimit() const
{
    return _LineLimit;
}

void PrintManager::SetTypingSpeed(ETypingSpeed NewSpeed)
{
    if (NewSpeed > ETypingSpeed::MAX || NewSpeed <= ETypingSpeed::NONE) NewSpeed = ETypingSpeed::Slow;
    _CurrentSpeed = NewSpeed;
}