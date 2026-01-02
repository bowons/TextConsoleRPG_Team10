#pragma once
#include "../Singleton.h"
#include <string>
#include <vector>

class InputManager : public Singleton<InputManager>
{
public:
    // 단순 문자열 입력
    std::string GetInput(const std::string& Prompt);
    // 정수 입력, 지정된 범위내의 값 받기
    int GetIntInput(const std::string& Prompt, int Min, int Max);
    // 정해진 옵션들 중 하나 입력받기
    std::string GetStringInput(const std::string& Prompt, const std::vector<std::string>& ValidOptions);
    // 정해둔 문자들 중에서 입력받기
    char GetCharInput(const std::string& Prompt, const std::string& ValidChars);
    // Yes or No 입력 받기
    bool GetYesNoInput(const std::string& Prompt);
    
private:
    // 입력 버퍼 비우기
    void ClearInputBuffer();
    // UTF-8 입력 받기
    std::string GetUTFInput();
};