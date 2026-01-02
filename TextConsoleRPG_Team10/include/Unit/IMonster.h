#pragma once
#include "ICharacter.h"
#include "../../include/Item/IItem.h"
#include <tuple>
#include <memory>

class IMonster : public ICharacter
{
protected:
    std::string _Stage;
public:
    virtual std::tuple<int, int, std::unique_ptr<IItem>> DropReward() = 0;
    virtual ~IMonster() {}
};
