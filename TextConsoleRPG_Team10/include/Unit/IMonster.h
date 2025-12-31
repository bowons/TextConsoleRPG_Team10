#pragma once
#include "ICharacter.h"
#include "../../include/Item/IItem.h"
#include <tuple>

class IMonster : public ICharacter
{
public:
    virtual tuple<int, int, IItem*> DropReward() = 0;
    virtual ~IMonster() {}
};
