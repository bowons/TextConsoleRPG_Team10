#pragma once
#include "ICharacter.h"
#include <tuple>

class IMonster : public ICharacter
{
public:
    virtual tuple<int, int, unique_ptr<IItem>> DropReward() = 0;
    virtual ~IMonster() {}
};
