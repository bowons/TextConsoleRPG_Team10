#pragma once
#include "../Singleton.h"
#include <vector>
#include <memory>
#include <string>
#include <tuple>
#include <map>

class IItem;
class Player;
class ItemFactory;

struct ItemStock {
    std::unique_ptr<IItem> StoredItem;
    int _StockCount;
    std::string AsciiFile;// 아이템 이미지 경로
};

// 상점 아이템 정보 (UI 표시용)
struct ShopItemInfo {
    std::string name;
    std::string description;
    int price;
    int stock;
    std::string asciiFile;  // 아이템 이미지 경로
};

// 거래 결과 반환 타입
// 사용 예시: auto [success, message, goldChange, itemName] = BuyItem(player, 0);
// - success: 거래 성공 여부 (true/false)
// - message: 실패 이유 또는 성공 메시지
// - goldChange: 골드 변동량 (구매 시 음수, 판매 시 양수)
// - itemName: 거래한 아이템 이름
using TransactionResult = std::tuple<bool, std::string, int, std::string>;

class ShopManager : public Singleton<ShopManager>
{
private:
    std::vector<ItemStock> _SellList;  // 판매 목록
    std::unique_ptr<ItemFactory> _ItemFactory;
    std::map<std::string, std::string> _ItemDescriptions;  // 아이템 ID -> 설명 매핑

protected:
    ShopManager();
    friend class Singleton<ShopManager>;

    ShopManager(const ShopManager&) = delete;
    ShopManager& operator=(const ShopManager&) = delete;

public:
// 판매 리스트 초기화 (CSV에서 로드)
    // 반환: 성공 시 true, 실패 시 false
 bool ReopenShop(const std::string& csvFileName = "Items.csv");

    // 판매 목록 정보 반환 (UI에서 표시)
    std::vector<ShopItemInfo> GetShopItems() const;
    
    // 아이템 구매
    // 반환: tuple<성공여부, 메시지, 골드변동(-), 아이템명>
    TransactionResult BuyItem(Player* player, int idx);
 
    // 아이템 판매
    // 반환: tuple<성공여부, 메시지, 골드변동(+), 아이템명>
    TransactionResult SellItem(Player* player, int slotIdx);

    // 판매 목록 크기 반환
    size_t GetSellListSize() const { return _SellList.size(); }
    
    // 특정 인덱스의 아이템 이미지 경로 반환
    std::string GetItemAsciiFile(int idx) const;
};
