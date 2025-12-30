#include <iostream>
#include "../../include/Manager/InputManager.h"
#include "../../include/Manager/PrintManager.h"

using namespace std;

string InputManager::GetInput(string Prompt)
{
    string str = TrashStr;
    PrintManager::GetInstance()->PrintLog(Prompt);
    cin >> str;
    return str;
}

int InputManager::GetIntInput(string Prompt, int Min, int Max)
{
    int input = TrashInt;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        cin >> input;
        if (cin.fail())
        {
            string Msg = "유효하지 않은 입력입니다.\n";
            PrintManager::GetInstance()->PrintLog(Msg, ELogImportance::WARNING);
            cin.clear();
            cin.ignore(10000000, '\n');
        }
        else if (input<Min || input>Max)
        {
            string Msg = "지정된 범우를 벗어나는 입력입니다.\n";
            PrintManager::GetInstance()->PrintLog(Msg, ELogImportance::WARNING);
        }
        else
        {
            return input;
        }
    }
}

string InputManager::GetStringInput(string Prompt, vector<string> ValidOptions)
{
    string str = TrashStr;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        for (const string& option : ValidOptions)
        {
            if (str == option)
            {
                return str;
            }
        }
        string Msg = "유효하지 않은 입력입니다: ";
        Msg.append(str);
        PrintManager::GetInstance()->PrintLog(Msg,ELogImportance::WARNING);
    }
}

char InputManager::GetCharInput(string Prompt, string validChars)
{
    char ch = TrashCh;
    while (true)
    {
        PrintManager::GetInstance()->PrintLog(Prompt);
        for (const char& option : validChars)
        {
            if (ch == option)
            {
                return ch;
            }
        }
        string Msg = "유효하지 않은 입력입니다: ";
        Msg+=ch;
        PrintManager::GetInstance()->PrintLog(Msg, ELogImportance::WARNING);
    }
}

bool InputManager::GetYesNoInput(string Prompt)
{
    string input = "";
    while (true)
    {
        if (input == "YES" || input == "Yes" || input == "yse")
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
            Msg.append(input);
            PrintManager::GetInstance()->PrintLog(Msg, ELogImportance::WARNING);
        }
    }
}
