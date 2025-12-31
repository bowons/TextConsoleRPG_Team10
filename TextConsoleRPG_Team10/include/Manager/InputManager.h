#pragma once
#include "../Singleton.h"
#include <string>
#include <vector>

using namespace std;

class InputManager : public Singleton<InputManager>
{
private:
    string TrashStr = "TrashHsarT";
    int TrashInt = -1005222;
    char TrashCh = '~';

public:
    string GetInput(string Prompt);
    int GetIntInput(string Prompt, int Min, int Max);
    string GetStringInput(string Prompt, vector<string> ValidOptions);
    char GetCharInput(string Prompt, string validChars);
    bool GetYesNoInput(string Prompt);
};
