#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/AttackUp.h"
#include "../../include/Unit/Player.h"
#include <memory>

using namespace std;

void ShopManager::ReopenShop()
{
    // unique_ptr이므로 자동으로 메모리 해제됨
    _SellList.clear();

    // 프로토타입 생성 및 재고 설정
    _SellList.push_back({ std::make_unique<HealPotion>(), 10 });
    _SellList.push_back({ std::make_unique<AttackUp>(), 5 });
}

void ShopManager::PrintShop()
{
    PrintManager::GetInstance()->PrintLogLine("===== 상점에 오신 것을 환영합니다! =====");
    PrintManager::GetInstance()->PrintLogLine("오늘의 판매 목록을 안내해드릴게요.");

    for (size_t i = 0; i < _SellList.size(); ++i)
    {
        const ItemStock& stock = _SellList[i];
        if (!stock.prototype) continue;

        std::string itemName = stock.prototype->GetName();
        int price = stock.prototype->GetPrice();

        std::string msg = std::to_string(i) + ". " + itemName +
            " | 가격: " + std::to_string(price) +
            "골드 | 재고: " + std::to_string(stock.count) + "개";
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
    if (stock.count <= 0)
    {
        PrintManager::GetInstance()->PrintLogLine("죄송합니다. 해당 상품은 품절입니다.");
        return false;
    }

    if (!stock.prototype)
    {
        PrintManager::GetInstance()->PrintLogLine("상품 정보가 올바르지 않습니다.", ELogImportance::WARNING);
        return false;
    }

    int Price = stock.prototype->GetPrice();
    std::string ItemName = stock.prototype->GetName();

    // 플레이어의 골드 확인
    if (Player->GetGold() < Price)
    {
        PrintManager::GetInstance()->PrintLogLine("소지금이 부족합니다. (" + std::to_string(Price) + "골드 필요)");
        return false;
    }

    // Clone을 사용하여 새 아이템 생성 (타입 판별 불필요)
    std::unique_ptr<IItem> Item = stock.prototype->Clone();

    // Inventory에 추가
    int remain;
    bool success = Player->GetInventory().AddItem(std::move(Item), 1, remain);
    if (success && remain == 0)
    {
        stock.count--;
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

int ShopManager::SellItem(Player* Player, int slotIdx)
{
    if (!Player) {
        return 0;
    }
    return 0;
}
