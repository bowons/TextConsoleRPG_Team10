#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Factory/ItemFactory.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Unit/Player.h"
#include "../../include/Config.h"
#include <memory>

using namespace std;

ShopManager::ShopManager()
{
    _ItemFactory = std::make_unique<ItemFactory>();
}

void ShopManager::ReopenShop(const std::string& csvFileName)
{
    _SellList.clear();

    // CSV에서 아이템 데이터 로드
    auto itemDataList = DataManager::GetInstance()->LoadItemData(csvFileName);

    if (itemDataList.empty())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "상점 데이터를 불러올 수 없습니다. 기본 상품으로 준비합니다.",
            ELogImportance::WARNING
        );

        // 기본 아이템으로 상점 구성
        _SellList.push_back({ std::make_unique<HealPotion>(), 10 });
        _SellList.push_back({ std::make_unique<AttackUp>(), 5 });
        return;
    }

    // CSV 데이터 기반으로 상점 재고 구성
    for (const auto& data : itemDataList)
    {
        // Factory::Create 사용
        auto item = _ItemFactory->Create(data.ItemType);
        if (item)
        {
            item->SetPrice(data.Price);
            _SellList.push_back({ std::move(item), data.Stock });
        }
        else
        {
            PrintManager::GetInstance()->PrintLogLine(
                "알 수 없는 아이템 타입: " + data.ItemType,
                ELogImportance::WARNING
            );
        }
    }

    PrintManager::GetInstance()->PrintLogLine(
        "상점에 " + std::to_string(_SellList.size()) + "종류의 상품이 입고되었습니다.",
        ELogImportance::DISPLAY
    );
}

void ShopManager::PrintShop()
{
    PrintManager::GetInstance()->PrintLogLine("===== 상점에 오신 것을 환영합니다! =====");
    PrintManager::GetInstance()->PrintLogLine("오늘의 판매 목록을 안내해드릴게요.");

    for (size_t i = 0; i < _SellList.size(); ++i)
    {
        const ItemStock& stock = _SellList[i];
        if (!stock.StoredItem) continue;

        std::string itemName = stock.StoredItem->GetName();
        int price = stock.StoredItem->GetPrice();

        std::string msg = std::to_string(i) + ". " + itemName +
            " | 가격: " + std::to_string(price) +
            "골드 | 재고: " + std::to_string(stock._StockCount) + "개";
        PrintManager::GetInstance()->PrintLogLine(msg);
    }

    PrintManager::GetInstance()->PrintLogLine("필요한 물건이 있으신가요? 선택해 주세요!");
}

bool ShopManager::BuyItem(Player* Player, int idx)
{
    if (!Player || idx < 0 || idx >= _SellList.size())
    {
        PrintManager::GetInstance()->PrintLogLine("구매할 수 없는 상품입니다.", ELogImportance::WARNING);
        return false;
    }

    ItemStock& stock = _SellList[idx];
    if (stock._StockCount <= 0)
    {
        PrintManager::GetInstance()->PrintLogLine("죄송합니다. 해당 상품은 품절입니다.");
        return false;
    }

    if (!stock.StoredItem)
    {
        PrintManager::GetInstance()->PrintLogLine("상품 정보가 올바르지 않습니다.", ELogImportance::WARNING);
        return false;
    }

    int Price = stock.StoredItem->GetPrice();
    std::string ItemName = stock.StoredItem->GetName();

    // 플레이어의 골드 확인
    if (Player->GetGold() < Price)
    {
        PrintManager::GetInstance()->PrintLogLine("소지금이 부족합니다. (" + std::to_string(Price) + "골드 필요)");
        return false;
    }

    // Clone을 사용하여 새 아이템 생성 (타입 판별 불필요)
    std::unique_ptr<IItem> Item = stock.StoredItem->Clone();

    // Inventory에 추가
    int remain;
    bool success = Player->GetInventory().AddItem(std::move(Item), 1, remain);
    if (success && remain == 0)
    {
        stock._StockCount--;
        Player->ModifyGold(-Price);
        PrintManager::GetInstance()->PrintLogLine(ItemName + "을(를) 구매하셨습니다! 남은 골드: " + std::to_string(Player->GetGold()));
        return true;
    }
    else
    {
        PrintManager::GetInstance()->PrintLogLine("인벤토리에 공간이 부족합니다.");
        return false;
    }
}

int ShopManager::SellItem(Player* Player, int SlotIdx)
{
    if (!Player) {
        return 0;
    }

    Inventory& Inventory = Player->GetInventory();

    int Amount = Inventory.GetSlotAmount(SlotIdx);
    if (Amount <= 0) {
        PrintManager::GetInstance()->PrintLogLine("해당 슬롯에 아이템이 없습니다.", ELogImportance::WARNING);
        return 0;
    }

    IItem* Item = Inventory.GetItemAtSlot(SlotIdx);
    if (!Item)
    {
        PrintManager::GetInstance()->PrintLogLine("잘못된 슬롯입니다.", ELogImportance::WARNING);
        return 0;
    }

    // 상점 목록에서 해당 타입의 아이템 가격 찾기
    int SellPrice = static_cast<int>(Item->GetPrice() * 0.6);
    if (SellPrice <= 0)
    {
        PrintManager::GetInstance()->PrintLogLine("이 아이템은 판매할 수 없습니다.", ELogImportance::WARNING);
        return 0;
    }

    // 아이템 제거 및 골드 지급
    if (Inventory.RemoveItem(SlotIdx, 1))
    {
        Player->ModifyGold(SellPrice);
        PrintManager::GetInstance()->PrintLogLine(
            Item->GetName() + "을(를) 판매하여 " + std::to_string(SellPrice) + "골드를 획득했습니다!"
        );
        return SellPrice;
    }

    return 0;
}
