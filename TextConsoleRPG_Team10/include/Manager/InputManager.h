#pragma once
#include "../Singleton.h"
#include <string>
#include <vector>

using namespace std;

class InputManager : public Singleton<InputManager>
{
public:
    // 단순 문자열 입력
    string GetInput(const string& Prompt);
    // 정수 입력, 지정된 범위내의 값 받기
    int GetIntInput(const string& Prompt, int Min, int Max);
    // 정해진 옵션들 중 하나 입력받기
    string GetStringInput(const string& Prompt, const vector<string>& ValidOptions);
    // 정해둔 문자들 중에서 입력받기
    char GetCharInput(const string& Prompt, const string& ValidChars);
    // Yes or No 입력 받기
    bool GetYesNoInput(const string& Prompt);

private:
    // 입력 버퍼 비우기
    void ClearInputBuffer();
};