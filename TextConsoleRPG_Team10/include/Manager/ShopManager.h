#pragma once
#include "../Singleton.h"
#include <vector>
#include <memory>
#include <string>

class IItem;
class Player;
class ItemFactory;
struct ItemStock {
    std::unique_ptr<IItem> StoredItem;  // unique_ptr로 자동 메모리 관리
    int _StockCount;
};

class ShopManager : public Singleton<ShopManager>
{
private:
    std::vector<ItemStock> _SellList;  // 판매 목록 (프로토타입 + 개수)
    std::unique_ptr<ItemFactory> _ItemFactory;  // ShopManager가 ItemFactory를 소유
    
protected:
    ShopManager();
    friend class Singleton<ShopManager>;

    ShopManager(const ShopManager&) = delete;
    ShopManager& operator=(const ShopManager&) = delete;

public:
    // 판매 리스트 초기화
    void ReopenShop(const std::string& csvFileName = "Items.csv");
    // 판매 리스트 출력
    void PrintShop();

    bool BuyItem(Player* Player, int Idx);
    int SellItem(Player* Player, int SlotIdx);
};
