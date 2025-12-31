#include "../../include/Manager/ShopManager.h"
#include <iostream>
using namespace std;

ShopManager::ShopManager()
{
    // Implementation needed
    // 판매 리스트 초기화 필요
}
void ShopManager::PrintShop()
{
    // Implementation needed
    cout << "Shop" << endl;
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
