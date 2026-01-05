#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <conio.h>
#include "../../include/Manager/InputManager.h"
#include "../../include/Manager/PrintManager.h"

// 단순 문자열 입력
std::string InputManager::GetInput(const std::string& Prompt)
{
    std::string str = "";
    PrintManager::GetInstance()->PrintLog(Prompt);
    return GetUTFInput();
}

// 특정 좌표에서 문자열 입력 (UI용)
std::string InputManager::GetInputAt(int x, int y, int maxLength, bool showCursor)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    // 커서를 지정 위치로 이동
    COORD pos = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(hConsole, pos);

    // 커서 표시 설정
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    bool originalCursorVisible = cursorInfo.bVisible;
    cursorInfo.bVisible = showCursor;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // UTF-8 입력 받기
    wchar_t wbuffer[1024];
    DWORD charactersRead = 0;

    if (!ReadConsoleW(hInput, wbuffer, 1024, &charactersRead, NULL))
    {
        // 커서 원래대로 복원
        cursorInfo.bVisible = originalCursorVisible;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        return "";
    }

    std::wstring wstr(wbuffer, charactersRead);

    // 개행 문자 제거
    while (!wstr.empty() && (wstr.back() == L'\r' || wstr.back() == L'\n'))
    {
        wstr.pop_back();
    }

    if (wstr.empty())
    {
        // 커서 원래대로 복원
        cursorInfo.bVisible = originalCursorVisible;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        return "";
    }

    // UTF-8로 변환
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string utf8Str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &utf8Str[0], size_needed, NULL, NULL);

    // 길이 제한 (시각적 칸 수)
    int visualLength = 0;
    size_t i = 0;
    std::string result;

    while (i < utf8Str.length() && visualLength < maxLength)
    {
        unsigned char ch = static_cast<unsigned char>(utf8Str[i]);
        int charLen = 1;
        int charWidth = 1;

        if (ch >= 0x80)
        {
            if ((ch & 0xE0) == 0xE0) charLen = 3; // 한글 (3바이트)
            else if ((ch & 0xC0) == 0xC0) charLen = 2;
            charWidth = 2;  // 한글은 2칸
        }

        if (visualLength + charWidth > maxLength)
        {
            break;
        }

        result += utf8Str.substr(i, charLen);
        visualLength += charWidth;
        i += charLen;
    }

    // 커서 원래대로 복원
    cursorInfo.bVisible = originalCursorVisible;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    return result;
}

// 정수 입력, 지정된 범위내의 값 받기
int InputManager::GetIntInput(const std::string& Prompt, int Min, int Max)
{
    int input = 0;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        std::cin >> input;

        if (std::cin.fail())  // 정수가 아닌 것을 입력
        {
            std::string Msg = "숫자만 입력 가능합니다.\n";
            // std::string Msg = "Only numbers can be entered.";
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
            ClearInputBuffer();
        }
        else if (input < Min || input > Max)  // 범위를 벗어남
        {
            std::string Msg = "범위를 벗어났습니다.\n";
            // std::string Msg = "Out of range.";
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
            ClearInputBuffer();
        }
        else
        {
            ClearInputBuffer();
            return input;
        }
    }
}

// 정해진 옵션들 중 하나 입력받기
std::string InputManager::GetStringInput(const std::string& Prompt, const std::vector<std::string>& ValidOptions)
{
    std::string str = "";
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        // getline(std::cin, str);
        str = GetUTFInput();
        for (const std::string& option : ValidOptions)
        {
            if (str == option)
            {
                return str;
            }
        }
        std::string Msg = "유효하지 않은 입력입니다: ";
        // std::string Msg = "Invalid input: ";
        Msg.append(str);
        PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
    }
}

// 정해둔 문자들 중에서 입력받기
char InputManager::GetCharInput(const std::string& Prompt, const std::string& ValidChars)
{
    std::string str;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        // getline(std::cin, str);
        str = GetUTFInput();

        // 빈 문자열이거나 2글자 이상 입력 시 에러 처리
        if (str.empty() || str.length() > 1)
        {
            std::string Msg = "유효하지 않은 입력입니다.\n";
            // std::string Msg = "Invalid input: ";
            Msg.append(str);
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
            ClearInputBuffer();
            continue;
        }

        for (const char& option : ValidChars)
        {
            if (str[0] == option)
            {
                return str[0];
            }
        }
        std::string Msg = "유효하지 않은 입력입니다: ";
        //std::string Msg = "Invalid input: ";
        Msg += str[0];
        PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
    }
}

// Yes or No 입력 받기
bool InputManager::GetYesNoInput(const std::string& Prompt)
{
    std::string input = "";
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        getline(std::cin, input);

        if (input == "YES" || input == "Yes" || input == "yes")
        {
            return true;
        }
        else if (input == "NO" || input == "No" || input == "no")
        {
            return false;
        }
        else
        {
            std::string Msg = "유효하지 않은 입력입니다: ";
            // std::string Msg = "Invalid input: ";
            Msg.append(input);
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
        }
    }
}

void InputManager::ClearInputBuffer()
{
    if (std::cin.fail() || std::cin.rdbuf()->in_avail() > 0)
    {
        std::cin.clear();
        // 개행 문자가 나올 때가지 버퍼 비우기
        std::cin.ignore(10000000, '\n');
    }
}

// 이석준님이 공유해주신 UTF 입력 받는 함수입니다.
std::string InputManager::GetUTFInput() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    wchar_t wbuffer[1024];
    DWORD charactersRead = 0;
    if (!ReadConsoleW(hInput, wbuffer, 1024, &charactersRead, NULL)) {
        return "";
    }
    std::wstring wstr(wbuffer, charactersRead);
    while (!wstr.empty() && (wstr.back() == L'\r' || wstr.back() == L'\n')) {
        wstr.pop_back();
    }
    if (wstr.empty()) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string utf8Str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &utf8Str[0], size_needed, NULL, NULL);
    return utf8Str;
}

// ===== 논블로킹 입력 메서드 구현 =====

// 키가 눌렸는지 확인 (논블로킹)
bool InputManager::IsKeyPressed() const
{
    return _kbhit() != 0;
}

// 눌린 키 코드 가져오기 (논블로킹)
int InputManager::GetKeyCode()
{
    if (!IsKeyPressed())
    {
        return 0;  // 키 입력 없음
    }

    int keyCode = _getch();

    // 확장 키 처리 (방향키 등)
    // 224 (0xE0) 또는 0 (0x00)이면 두 번째 바이트를 읽어야 함
    if (keyCode == 224 || keyCode == 0)
    {
        keyCode = _getch();  // 실제 키 코드 읽기
    }

    return keyCode;
}

// 특정 키가 눌렸는지 확인 및 소비 (논블로킹)
bool InputManager::IsKeyDown(int keyCode)
{
    if (!IsKeyPressed())
    {
        return false;
    }

    int pressedKey = _getch();
    return pressedKey == keyCode;
}

// 특정 문자 키가 눌렸는지 확인 및 소비 (대소문자 구분 없음)
bool InputManager::IsCharPressed(char ch)
{
    if (!IsKeyPressed())
    {
        return false;
    }

    int pressedKey = _getch();

    // 대소문자 구분 없이 비교
    if (pressedKey >= 'A' && pressedKey <= 'Z')
    {
        pressedKey = pressedKey - 'A' + 'a';  // 소문자로 변환
    }

    char targetChar = ch;
    if (targetChar >= 'A' && targetChar <= 'Z')
    {
        targetChar = targetChar - 'A' + 'a';  // 소문자로 변환
    }

    return pressedKey == targetChar;
}