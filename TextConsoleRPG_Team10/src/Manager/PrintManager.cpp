#include <iostream>
#include "Windows.h"
#include "../../include/Manager/PrintManager.h"

// 로그 및 일반 출력 함수 - 개행 없음
void PrintManager::PrintLog(const std::string& Msg, ELogImportance Importance)
{
    ETextColor PrevColor = GetCurrentTextColor();
    switch (Importance)
    {
    // 일반 강조
    case ELogImportance::DISPLAY:
    {
        ChangeTextColor(ETextColor::YELLOW);
        std::cout << "[DISPLAY]: ";
    }
    break;
    // 심각함 강조 - 에러나 위험 표시에 사용
    case ELogImportance::WARNING:
    {
        ChangeTextColor(ETextColor::RED);
        std::cout << "[WARNING]: ";
    }
    break;
    default:
        break;
    }

    int LineCnt = Msg.length() / _LineLimit + 1;
    for (int i = 0; i < LineCnt; ++i)
    {
        std::string MsgStr = Msg.substr(i * _LineLimit, _LineLimit);
        std::cout << MsgStr;
        if (i != LineCnt -1)
        {
            EndLine();
        }
    }

    ChangeTextColor(PrevColor);
}

// 로그 및 일반 출력 함수 - 개행 있음
void PrintManager::PrintLogLine(const std::string& Msg, ELogImportance Importance)
{
    PrintLog(Msg, Importance);
    EndLine();
}

// 타이핑 효과 적용 출력 함수 - 개행 없음
void PrintManager::PrintWithTyping(const std::string& Msg)
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

    for (const char& ch : Msg)
    {
        std::cout << ch;
        Sleep(ch == ' ' ? _Interval : _Interval *2);
        // 줄당 글자 제한수에 걸리면 다음 줄로 개행
        ++_CurrentCharCnt;
        if (_CurrentCharCnt >= _LineLimit)
        {
            EndLine();
        }
    }
}

// 타이핑 효과 적용 출력 함수 - 개행 있음
void PrintManager::PrintWithTypingLine(const std::string& Msg)
{
    PrintWithTyping(Msg);
    EndLine();
}

void PrintManager::PrintColorText(const std::string& Msg, ETextColor Color)
{
    ETextColor PrevColor = GetCurrentTextColor();
    ChangeTextColor(Color);
    PrintLog(Msg);
    ChangeTextColor(PrevColor);
}

// 텍스트 색상 변경
void PrintManager::ChangeTextColor(ETextColor NewTextColor)
{
    int ColorIndex = static_cast<int>(NewTextColor);
    if (NewTextColor >= ETextColor::MAX)
    {
        std::cout << "\nInvalid Text Color Index: " << static_cast<int>(NewTextColor);
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
    std::cout << '\n';
    _CurrentCharCnt =0 ;
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