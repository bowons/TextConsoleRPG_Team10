#pragma once
#include "../Singleton.h"
#include <vector>

using namespace std;

class IItem;
class Player;

class ShopManager : public Singleton<ShopManager>
{
    friend class Singleton<ShopManager>;
protected:
    ShopManager() {};

private:
    vector<IItem*> _SellList;
    vector<IItem*> _ResellList;

public:
    void PrintShop();
    bool BuyItem(Player* Player, int Idx);
    int SellItem(Player* Player, int SlotIdx);
};
