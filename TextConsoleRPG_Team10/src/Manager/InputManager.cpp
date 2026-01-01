#include <iostream>
#include "../../include/Manager/InputManager.h"
#include "../../include/Manager/PrintManager.h"

using namespace std;

// 단순 문자열 입력
string InputManager::GetInput(const string& Prompt)
{
    string str = "";
    PrintManager::GetInstance()->PrintLog(Prompt);
    getline(cin, str);  // getline은 입력버퍼를 비우지 않아도 됨.

    return str;
}

// 정수 입력, 지정된 범위내의 값 받기
int InputManager::GetIntInput(const string& Prompt, int Min, int Max)
{
    int input = 0;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        cin >> input;

        if (cin.fail())  // 정수가 아닌 것을 입력
        {
            string Msg = "숫자만 입력 가능합니다.\n";
            // string Msg = "Only numbers can be entered.";
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
            ClearInputBuffer();
        }
        else if (input < Min || input > Max)  // 범위를 벗어남
        {
            string Msg = "범위를 벗어났습니다.\n";
            // string Msg = "Out of range.";
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
string InputManager::GetStringInput(const string& Prompt, const vector<string>& ValidOptions)
{
    string str = "";
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        getline(cin, str);
        for (const string& option : ValidOptions)
        {
            if (str == option)
            {
                return str;
            }
        }
        string Msg = "유효하지 않은 입력입니다: ";
        // string Msg = "Invalid input: ";
        Msg.append(str);
        PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
    }
}

// 정해둔 문자들 중에서 입력받기
char InputManager::GetCharInput(const string& Prompt, const string& ValidChars)
{
    string str;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        getline(cin, str);

        // 빈 문자열이거나 2글자 이상 입력 시 에러 처리
        if (str.empty() || str.length() > 1)
        {
            string Msg = "유효하지 않은 입력입니다.\n";
            // string Msg = "Invalid input: ";
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
        string Msg = "유효하지 않은 입력입니다: ";
        //string Msg = "Invalid input: ";
        Msg += str[0];
        PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
    }
}

// Yes or No 입력 받기
bool InputManager::GetYesNoInput(const string& Prompt)
{
    string input = "";
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        getline(cin, input);

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
            string Msg = "유효하지 않은 입력입니다: ";
            // string Msg = "Invalid input: ";
            Msg.append(input);
            PrintManager::GetInstance()->PrintLogLine(Msg, ELogImportance::WARNING);
        }
    }
}

void InputManager::ClearInputBuffer()
{
    if (cin.fail() || cin.rdbuf()->in_avail() > 0)
    {
        cin.clear();
        // 개행 문자가 나올 때가지 버퍼 비우기
        cin.ignore(10000000, '\n');
    }
}
