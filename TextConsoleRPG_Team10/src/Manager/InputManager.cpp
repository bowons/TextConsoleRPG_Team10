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

    // 입력 버퍼 플러시 (이전 입력 제거)
    FlushConsoleInputBuffer(hInput);

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
    wchar_t wbuffer[1024] = { 0 };  // 버퍼 초기화
    DWORD charactersRead = 0;

    if (!ReadConsoleW(hInput, wbuffer, 1024, &charactersRead, NULL))
    {
        // 커서 원래대로 복원
        cursorInfo.bVisible = originalCursorVisible;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        // 입력 버퍼 플러시
        FlushConsoleInputBuffer(hInput);
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
        // 입력 버퍼 플러시
        FlushConsoleInputBuffer(hInput);
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

    // 입력 버퍼 플러시 (남은 입력 제거)
    FlushConsoleInputBuffer(hInput);

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

    // 입력 버퍼 플러시 (이전 입력 제거)
    FlushConsoleInputBuffer(hInput);

    wchar_t wbuffer[1024] = { 0 };  // 버퍼 초기화
    DWORD charactersRead = 0;

    if (!ReadConsoleW(hInput, wbuffer, 1024, &charactersRead, NULL)) {
        FlushConsoleInputBuffer(hInput);
        return "";
    }

    std::wstring wstr(wbuffer, charactersRead);

    while (!wstr.empty() && (wstr.back() == L'\r' || wstr.back() == L'\n')) {
        wstr.pop_back();
    }

    if (wstr.empty()) {
        FlushConsoleInputBuffer(hInput);
        return "";
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string utf8Str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &utf8Str[0], size_needed, NULL, NULL);

    // 입력 버퍼 플러시 (남은 입력 제거)
    FlushConsoleInputBuffer(hInput);

    return utf8Str;
}

// ===== 논블로킹 입력 메서드 구현 =====

// 키가 눌렸는지 확인 (논블로킹)
bool InputManager::IsKeyPressed() const
{
    // Windows Console Input 이벤트를 직접 확인
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD eventCount = 0;

    if (!GetNumberOfConsoleInputEvents(hInput, &eventCount))
    {
        return false;
    }

    if (eventCount == 0)
    {
        return false;
    }

    // 입력 이벤트가 있는지 확인 (Peek)
    INPUT_RECORD inputRecord[128];
    DWORD eventsRead = 0;

    if (!PeekConsoleInput(hInput, inputRecord, 128, &eventsRead))
    {
        return false;
    }

    // 실제 키 입력 이벤트가 있는지 확인 (한글 입력 제외)
    for (DWORD i = 0; i < eventsRead; ++i)
    {
        if (inputRecord[i].EventType == KEY_EVENT &&
            inputRecord[i].Event.KeyEvent.bKeyDown)
        {
            // 가상 키 코드 확인
            WORD vkCode = inputRecord[i].Event.KeyEvent.wVirtualKeyCode;

            // IME 관련 키 코드 필터링 (한글 입력 관련)
            if (vkCode == VK_PROCESSKEY || vkCode == VK_HANGUL ||
                vkCode == VK_HANJA || vkCode == VK_IME_ON ||
                vkCode == VK_IME_OFF)
            {
                // 한글 입력 이벤트는 무시하고 버퍼에서 제거
                ReadConsoleInput(hInput, inputRecord, 1, &eventsRead);
                continue;
            }

            // ASCII 제어 문자 또는 유효한 가상 키만 허용
            if (vkCode > 0 && vkCode < 255)
            {
                return true;
            }
        }
    }

    return false;
}

// 눌린 키 코드 가져오기 (논블로킹)
int InputManager::GetKeyCode()
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD inputRecord;
    DWORD eventsRead = 0;

    while (true)
    {
        DWORD eventCount = 0;
        if (!GetNumberOfConsoleInputEvents(hInput, &eventCount) || eventCount == 0)
        {
            return 0;  // 키 입력 없음
        }

        if (!ReadConsoleInput(hInput, &inputRecord, 1, &eventsRead))
        {
            return 0;
        }

        // 키 이벤트만 처리
        if (inputRecord.EventType != KEY_EVENT)
        {
            continue;
        }

        // 키를 누르는 이벤트만 처리 (떼는 이벤트 무시)
        if (!inputRecord.Event.KeyEvent.bKeyDown)
        {
            continue;
        }

        WORD vkCode = inputRecord.Event.KeyEvent.wVirtualKeyCode;

        // IME 관련 키 무시
        if (vkCode == VK_PROCESSKEY || vkCode == VK_HANGUL ||
            vkCode == VK_HANJA || vkCode == VK_IME_ON ||
            vkCode == VK_IME_OFF || vkCode == 0)
        {
            continue;  // 한글 입력 무시
        }

        // 확장 키 처리 (방향키 등)
        if (vkCode == VK_LEFT || vkCode == VK_RIGHT ||
            vkCode == VK_UP || vkCode == VK_DOWN)
        {
            // 방향키는 특별한 코드로 반환
            switch (vkCode)
            {
            case VK_LEFT:  return 75;   // 왼쪽 화살표
            case VK_RIGHT: return 77;   // 오른쪽 화살표
            case VK_UP:    return 72;   // 위쪽 화살표
            case VK_DOWN:  return 80;   // 아래쪽 화살표
            }
        }

        // ASCII 문자 또는 특수 키 반환
        if (vkCode == VK_RETURN)
        {
            return VK_RETURN;  // Enter
        }
        else if (vkCode == VK_ESCAPE)
        {
            return VK_ESCAPE;  // ESC
        }
        else if (vkCode == VK_SPACE)
        {
            return VK_SPACE;  // Space
        }
        else if (vkCode == VK_TAB)
        {
            return VK_TAB;  // Tab
        }
        else if (vkCode >= 0x30 && vkCode <= 0x5A)  // 0-9, A-Z
        {
            // ASCII 문자
            char asciiChar = inputRecord.Event.KeyEvent.uChar.AsciiChar;
            if (asciiChar > 0 && asciiChar < 128)
            {
                return static_cast<int>(asciiChar);
            }
        }

        // 기타 유효한 키 코드
        if (vkCode > 0 && vkCode < 255)
        {
            return static_cast<int>(vkCode);
        }
    }

    return 0;
}

// 특정 키가 눌렸는지 확인 및 소비 (논블로킹)
bool InputManager::IsKeyDown(int keyCode)
{
    if (!IsKeyPressed())
    {
        return false;
    }

    int pressedKey = GetKeyCode();
    return pressedKey == keyCode;
}

// 특정 문자 키가 눌렸는지 확인 및 소비 (대소문자 구분 없음)
bool InputManager::IsCharPressed(char ch)
{
    if (!IsKeyPressed())
    {
        return false;
    }

    int pressedKey = GetKeyCode();

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