#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Factory/ItemFactory.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/FairyEssence.h"
#include "../../include/Unit/Player.h"
#include "../../include/Item/Inventory.h"
#include "../../include/Config.h"
#include <memory>

using namespace std;

ShopManager::ShopManager()
{
    _ItemFactory = std::make_unique<ItemFactory>();
}

bool ShopManager::ReopenShop(const std::string& csvFileName)
{
    _SellList.clear();
    _ItemDescriptions.clear();

    // CSV에서 아이템 데이터 로드
    auto itemDataList = DataManager::GetInstance()->LoadItemData(csvFileName);

    if (itemDataList.empty())
    {
        // 기본 아이템으로 상점 구성
        _SellList.push_back({ std::make_unique<HealPotion>(), 8, "Items/Potion.txt" });
        _SellList.push_back({ std::make_unique<FairyEssence>(), 8, "Items/Potion.txt" });
        return false;  // 로드 실패
    }

    // CSV 데이터 기반으로 상점 재고 구성
    for (const auto& data : itemDataList)
    {
        // CreateFromData 사용 - CSV 데이터 기반 아이템 생성
        auto item = _ItemFactory->CreateFromData(data);
        if (item)
        {
            _SellList.push_back({ std::move(item), data.Stock, data.AsciiFile });
            _ItemDescriptions[data.ItemID] = data.Description;
        }
    }

    return true;  // 로드 성공
}

std::vector<ShopItemInfo> ShopManager::GetShopItems() const
{
    std::vector<ShopItemInfo> items;

    for (const auto& stock : _SellList)
    {
        if (!stock.StoredItem) continue;

        ShopItemInfo info;
        info.name = stock.StoredItem->GetName();
        info.price = stock.StoredItem->GetPrice();
        info.stock = stock._StockCount;
        info.asciiFile = stock.AsciiFile;

        // 설명 찾기 (ItemID를 키로 사용, 없으면 빈 문자열)
        auto it = _ItemDescriptions.find(stock.StoredItem->GetName());
        info.description = (it != _ItemDescriptions.end()) ? it->second : "";

        items.push_back(info);
    }

    return items;
}

std::string ShopManager::GetItemAsciiFile(int idx) const
{
    if (idx < 0 || idx >= static_cast<int>(_SellList.size()))
    {
        return "";
    }

    return _SellList[idx].AsciiFile;
}

TransactionResult ShopManager::BuyItem(Player* player, int idx)
{
    // 유효성 검사
    if (!player)
    {
        return { false, "플레이어 정보가 없습니다.", 0, "" };
    }

    if (idx < 0 || idx >= static_cast<int>(_SellList.size()))
    {
        return { false, "잘못된 상품 번호입니다.", 0, "" };
    }

    ItemStock& stock = _SellList[idx];

    if (stock._StockCount <= 0)
    {
        return { false, "품절된 상품입니다.", 0, "" };
    }

    if (!stock.StoredItem)
    {
        return { false, "상품 정보 오류입니다.", 0, "" };
    }

    int price = stock.StoredItem->GetPrice();
    std::string itemName = stock.StoredItem->GetName();

    // 골드 확인
    if (player->GetGold() < price)
    {
        return { false, "골드가 부족합니다. (" + std::to_string(price) + "G 필요)", 0, itemName };
    }

    // 인벤토리 접근 (bool + 포인터)
    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory))
    {
        return { false, "이 캐릭터는 아이템을 구매할 수 없습니다.", 0, itemName };
    }

    // 아이템 생성 및 인벤토리 추가
    std::unique_ptr<IItem> item = stock.StoredItem->Clone();
    int remain;
    bool addSuccess = inventory->AddItem(std::move(item), 1, remain);

    if (addSuccess && remain == 0)
    {
        stock._StockCount--;
        player->ModifyGold(-price);

        return { true, "구매 성공!", -price, itemName };
    }
    else
    {
        return { false, "인벤토리 공간이 부족합니다.", 0, itemName };
    }
}

TransactionResult ShopManager::SellItem(Player* player, int slotIdx)
{
    if (!player)
    {
        return { false, "플레이어 정보가 없습니다.", 0, "" };
    }

    // 인벤토리 접근 (bool + 포인터)
    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory))
    {
        return { false, "이 캐릭터는 아이템을 판매할 수 없습니다.", 0, "" };
    }

    int amount = inventory->GetSlotAmount(slotIdx);
    if (amount <= 0)
    {
        return { false, "해당 슬롯에 아이템이 없습니다.", 0, "" };
    }

    IItem* item = inventory->GetItemAtSlot(slotIdx);
    if (!item)
    {
        return { false, "잘못된 슬롯입니다.", 0, "" };
    }

    // 판매가 계산 (구매가의 60%)
    int sellPrice = static_cast<int>(item->GetPrice() * 0.6);
    if (sellPrice <= 0)
    {
        return { false, "판매할 수 없는 아이템입니다.", 0, "" };
    }

    std::string itemName = item->GetName();

    // 아이템 제거 및 골드 지급
    if (inventory->RemoveItem(slotIdx, 1))
    {
        player->ModifyGold(sellPrice);

        return { true, "판매 성공!", sellPrice, itemName };
    }

    return { false, "아이템 제거 실패.", 0, itemName };
}
