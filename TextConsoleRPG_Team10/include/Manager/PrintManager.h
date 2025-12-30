#pragma once
#include "../Singleton.h"
#include <string>

using namespace std;

enum class ETypingSpeed : int
{   
    NONE = -1,
    Slow = 0,
    Normal,
    Fast,
    MAX
};
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

enum class ELogImportance : int
{
    NONE = 0, DISPLAY, WARNING, MAX
};

class PrintManager : public Singleton<PrintManager>
{
private:
    ETypingSpeed _CurrentSpeed;
    int _LineLimit;

public:
    void PrintLog(string Msg, ELogImportance Importance = ELogImportance::NONE);
    void PrintWithTyping(string Msg);
    void ChangeTextColor(ETextColor NewTextColor = ETextColor::WHITE);
    ETextColor GetCurrentTextColor();
    void SetLineLimit(int Limit);
    int GetLineLimit();
    void SetTypingSpeed(ETypingSpeed NewSpeed);
};
