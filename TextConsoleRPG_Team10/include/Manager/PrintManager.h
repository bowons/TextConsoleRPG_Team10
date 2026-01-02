#pragma once
#include "../Singleton.h"
#include <string>

// 타이핑 속도 지정에 사용
enum class ETypingSpeed : int
{
    NONE = -1,
    Slow = 0,
    Normal,
    Fast,
    MAX
};

// 텍스트 색상 변경에 사용
enum class ETextColor : int
{
    NONE = -1,
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    YELLOW,
    LIGHT_GRAY,
    DARK_GRAY,
    LIGHT_BLUE,
    LIGHT_GREEN,
    LIGHT_CYAN,
    LIGHT_RED,
    LIGHT_MAGENTA,
    LIGHT_YELLOW,
    WHITE,
    MAX
};

// Log 출력 시 사용
enum class ELogImportance : int
{
    NONE = 0,  // 일반 출력
    DISPLAY,   // [DISPLAY] ~ 출력
    WARNING,   // [WARNING] ~ 출력
    MAX
};

class PrintManager : public Singleton<PrintManager>
{
private:
    // 현재 타이핑 속도
    ETypingSpeed _CurrentSpeed = ETypingSpeed::Normal;
    // 줄 당 글자 제한 수(= 줄 길이)
    int _LineLimit = 106;
    // 현재 줄에 출력된 글자 수
    int _CurrentCharCnt = 0;

public:
    // 로그 출력 및 일반 출력 함수
    void PrintLog(const std::string& Msg, ELogImportance Importance = ELogImportance::NONE);
    // 로그 출력 및 일반 출력 함수 + 개행
    void PrintLogLine(const std::string& Msg, ELogImportance Importance = ELogImportance::NONE);
    
    // 타이핑 효과를 적용한 출력 함수
    void PrintWithTyping(const std::string& Msg);
    // 타이핑 효과를 적용한 출력 함수 + 개행
    void PrintWithTypingLine(const std::string& Msg);

    void PrintColorText(const std::string& Msg, ETextColor Color);
    
    // 콘솔 텍스트 색상 변경 함수
    void ChangeTextColor(ETextColor NewTextColor = ETextColor::WHITE);
    // 현재 콘솔 텍스트 색상 반환
    const ETextColor GetCurrentTextColor();
    // 개행
    void EndLine();

    // 한 줄당 글자 제한 수 설정 함수
    void SetLineLimit(int Limit);
    // 한 줄당 글자 제한 수 반환 함수
    const int& GetLineLimit() const;
    // 타이핑 효과의 속도 설정 함수
    void SetTypingSpeed(ETypingSpeed NewSpeed);
}; 