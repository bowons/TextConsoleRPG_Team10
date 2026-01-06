#include "../../include/Manager/ShopManager.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/SoundPlayer.h"
#include "../../include/Factory/ItemFactory.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Item/HealPotion.h"
#include "../../include/Item/FairyEssence.h"
#include "../../include/Unit/Player.h"
#include "../../include/Item/Inventory.h"
#include "../../include/Config.h"
#include <memory>
#include <random>
#include <algorithm>

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

    std::vector<ItemData> availableItems;
    std::vector<float> weights;
    float totalWeight = 0.0f;

    // 판매 가능한 아이템과 그 가중치를 가져온다.
    for (const auto& data : itemDataList)
    {
        if (data.ShopAppearRate > 0.0f)
        {
            availableItems.push_back(data);
            weights.push_back(data.ShopAppearRate);
            totalWeight += data.ShopAppearRate;
        }
    }

    if (availableItems.empty())
    {
        // 판매 가능한 아이템이 없으면 기본 아이템으로 구성
        _SellList.push_back({ std::make_unique<HealPotion>(), 8, "Items/Potion.txt" });
        _SellList.push_back({ std::make_unique<FairyEssence>(), 8, "Items/Potion.txt" });
        return false;  // 로드 실패
    }

    std::vector<ItemData> selectedItems;
    std::vector<bool> used(availableItems.size(), false);
    int itemsToSelect = min(3, static_cast<int>(availableItems.size()));

    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 아이템을 무작위로 선택 - 가중치 누적합? 알고리즘이라고 한다
    // 게임 회사처럼 테이블에 있는 확률표대로 아이템을 뽑아내기!!
    for (int i = 0; i < itemsToSelect; ++i)
    {
        std::uniform_real_distribution<float> dist(0.0f, totalWeight);
        float randomValue = dist(gen);

        // 가중치 누적합으로 아이템 선택
        float currentSum = 0.0f;
        for (size_t j = 0; j < availableItems.size(); ++j)
        {
            if (used[j]) continue; // 이미 선택된 아이템은 건너뜀

            currentSum += weights[j];
            if (randomValue <= currentSum)
            {
                selectedItems.push_back(availableItems[j]); 
                used[j] = true;
                totalWeight -= weights[j]; // 선택된 아이템 가중치 제거
                break;
            }
        }
    }

    // 선택된 아이템으로 상점 재고 구성
    for (const auto& data : selectedItems)
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

    // 1. 동일 아이템 슬롯에 추가 가능 여부 확인
    int maxStack = stock.StoredItem->GetMaxCount();
    bool canAddToExisting = false;
    
    // 기존 슬롯에 추가 가능한지 확인
    for (int i = 0; i < 5; ++i)  // 인벤토리 최대 5칸 가정
    {
        IItem* slotItem = inventory->GetItemAtSlot(i);
        if (slotItem && typeid(*slotItem) == typeid(*stock.StoredItem))
        {
            int currentAmount = inventory->GetSlotAmount(i);
            if (currentAmount < maxStack)
            {
                canAddToExisting = true;
                break;
            }
        }
    }

    // 2. 빈 슬롯 확인 (기존 슬롯에 추가 불가능한 경우)
    bool hasEmptySlot = false;
    if (!canAddToExisting)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (inventory->GetSlotAmount(i) <= 0)
            {
                hasEmptySlot = true;
                break;
            }
        }
    }

    // 3. 추가 불가능한 경우 구매 거부 (골드 차감 X)
    if (!canAddToExisting && !hasEmptySlot)
    {
        return { false, "인벤토리 공간이 부족합니다.", 0, itemName };
    }

    // ===== 여기부터는 추가 가능한 경우만 진행 =====
    
    // 아이템 생성 및 인벤토리 추가
    std::unique_ptr<IItem> item = stock.StoredItem->Clone();
    int remain;
    bool addSuccess = inventory->AddItem(std::move(item), 1, remain);

    if (addSuccess && remain == 0)
    {
        // 성공적으로 추가됨 → 골드 차감 & 재고 감소
        stock._StockCount--;
        player->ModifyGold(-price);

        SoundPlayer::GetInstance()->PlaySFX("Item_Buy");
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

        SoundPlayer::GetInstance()->PlaySFX("Item_Sell");
        return { true, "판매 성공!", sellPrice, itemName };
    }

    return { false, "아이템 제거 실패.", 0, itemName };
}
