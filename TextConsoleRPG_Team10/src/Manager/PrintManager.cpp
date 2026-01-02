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
        _CurrentCharCnt += 11;
    }
    break;
    // 심각함 강조 - 에러나 위험 표시에 사용
    case ELogImportance::WARNING:
    {
        ChangeTextColor(ETextColor::RED);
        std::cout << "[WARNING]: ";
        _CurrentCharCnt += 11;
    }
    break;
    default:
        break;
    }

    for (int i = 0; i < Msg.length(); )
    {
        unsigned char ch = static_cast<unsigned char>(Msg[i]);
        int charLen = 1;
        int visualWidth = 1;

        // UTF-8 첫 바이트를 보고 이 글자가 몇 바이트인지 판별
        if (ch >= 0x80) {
            if ((ch & 0xE0) == 0xE0) charLen = 3;      // 한글 (3바이트)
            else if ((ch & 0xC0) == 0xC0) charLen = 2; // 기타 유니코드
            visualWidth = 2; // 한글 등 멀티바이트는 화면에서 2칸 차지
        }

        // 현재 칸수 + 이번 글자 폭이 제한을 넘으면 개행
        if (_CurrentCharCnt + visualWidth > _LineLimit)
        {
            EndLine();
        }

        // 한글 바이트 중간이 잘리는 현상을 방지하기 위해 하나씩 출력
        for (int j = 0; j < charLen && (i + j) < Msg.length(); ++j)
        {
            std::cout << Msg[i + j];
        }

        // 카운트 업데이트
        _CurrentCharCnt += visualWidth;
        i += charLen;
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
    int _Interval = GetIntervalTime();
    

    for (int i = 0; i < Msg.length(); )
    {
        unsigned char ch = static_cast<unsigned char>(Msg[i]);
        int charLen = 1;
        int visualWidth = 1;

        // UTF-8 첫 바이트를 보고 이 글자가 몇 바이트인지 판별
        if (ch >= 0x80) {
            if ((ch & 0xE0) == 0xE0) charLen = 3;      // 한글 (3바이트)
            else if ((ch & 0xC0) == 0xC0) charLen = 2; // 기타 유니코드
            visualWidth = 2; // 한글 등 멀티바이트는 화면에서 2칸 차지
        }

        // 현재 칸수 + 이번 글자 폭이 제한을 넘으면 개행
        if (_CurrentCharCnt + visualWidth > _LineLimit)
        {
            EndLine();
        }

        // 한글 바이트 중간이 잘리는 현상을 방지하기 위해 하나씩 출력
        for (int j = 0; j < charLen && (i + j) < Msg.length(); ++j)
        {
            std::cout << Msg[i + j];
        }

        // 카운트 업데이트
        _CurrentCharCnt += visualWidth;
        i += charLen;

        // 딜레이 주기
        Sleep(ch == ' ' ? _Interval : _Interval * 2);
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

int PrintManager::GetIntervalTime() const
{
    switch (_CurrentSpeed)
    {
    case ETypingSpeed::Slow:
    {
        return 100;
    }
    break;
    case ETypingSpeed::Normal:
    {
        return 50;
    }

    break;
    case ETypingSpeed::Fast:
    {
        return 20;
    }
    break;
    default:
        break;
    }
}
