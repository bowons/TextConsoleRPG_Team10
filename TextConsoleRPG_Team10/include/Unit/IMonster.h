#pragma once
#include "ICharacter.h"
#include <tuple>

class IMonster : public ICharacter
{
protected:
    string _Stage;
public:
    virtual tuple<int, int, unique_ptr<IItem>> DropReward() = 0;
    virtual ~IMonster() {}
};
