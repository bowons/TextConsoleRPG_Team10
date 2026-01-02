#pragma once
#include "../Singleton.h"
#include <vector>
#include <memory>

using namespace std;

class IItem;
class Player;

struct ItemStock {
    std::unique_ptr<IItem> prototype;  // unique_ptr로 자동 메모리 관리
    int count;
};

class ShopManager : public Singleton<ShopManager>
{
private:
    vector<ItemStock> _SellList;  // 판매 목록 (프로토타입 + 개수)
    
protected:
    ShopManager() = default;
    friend class Singleton<ShopManager>;

    ShopManager(const ShopManager&) = delete;
    ShopManager& operator=(const ShopManager&) = delete;

public:
    // 판매 리스트 초기화
    void ReopenShop();

    // 판매 리스트 출력
    void PrintShop();
    bool BuyItem(Player* Player, int Idx);
    int SellItem(Player* Player, int SlotIdx);
};
