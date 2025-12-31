#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"

using namespace std;

void ShopManager::ReopenShop()
{
    for (IItem* it : _SellList)
    {
        delete it;
    }
    _SellList.clear();
    
    _SellList.push_back(new HealPotion());
    _SellList.push_back(new AttackUp());
}

void ShopManager::PrintShop()
{
    // 판매 목록 출력

}

bool ShopManager::BuyItem(Player* Player, int idx)
{
    // Implementation needed
    if (!Player)
    {
        return false;
    }
    return false;
}

int ShopManager::SellItem(Player* Player, int slotIdx)
{
    // Implementation needed
    if (!Player)
    {
        return false;
    }
    return 0;
}
