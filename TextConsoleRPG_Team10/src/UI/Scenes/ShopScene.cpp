#include "../../../include/UI/Scenes/ShopScene.h"
#include "../../../include/UI/UIDrawer.h"
#include "../../../include/UI/Panel.h"
#include "../../../include/UI/TextRenderer.h"
#include "../../../include/UI/AsciiArtRenderer.h"
#include "../../../include/Manager/InputManager.h"
#include "../../../include/Manager/SceneManager.h"
#include "../../../include/Manager/GameManager.h"
#include "../../../include/Manager/ShopManager.h"
#include "../../../include/Manager/DataManager.h"
#include "../../../include/Manager/StageManager.h"
#include "../../../include/Manager/SoundPlayer.h"
#include "../../../include/Common/TextColor.h"
#include "../../../include/Unit/Player.h"
#include "../../../include/Item/Inventory.h"
#include "../../../include/Item/IItem.h"

ShopScene::ShopScene()
    : UIScene("Shop")
    , _SelectedItemIndex(0)
    , _IsBuyMode(true)
    , _PlayerInventorySlot(0)
{
}

ShopScene::~ShopScene()
{
}

void ShopScene::Enter()
{
    _Drawer->ClearScreen();
    _Drawer->RemoveAllPanels();
    _Drawer->Activate();
    _IsActive = true;

    _SystemLogs.clear();
    _SystemLogs.push_back("[상점 주인] 어서오게 용사. 통신에는 이상 없네, 원하는 물건을 골라보게.");
    _SystemLogs.push_back("");

    // 상점 재개장 (CSV에서 상품 로드)
    ShopManager::GetInstance()->ReopenShop("Items.csv");

    // 상점 BGM 재생
    SoundPlayer::GetInstance()->PlayBGM("BGM_Shop");
    // =============================================================================
    // 패널 레이아웃 (150x45 화면 기준)
    // =============================================================================

    // ===== 타이틀 패널 (상단) - Stage와 공통 =====
    Panel* titlePanel = _Drawer->CreatePanel("Title", 2, 1, 120, 5);
    titlePanel->SetBorder(true, ETextColor::WHITE);

    auto titleText = std::make_unique<TextRenderer>();
    titleText->AddLineWithColor("[ 에테르 환영거울 - 상점 통신중... ]",
        MakeColorAttribute(ETextColor::BLACK, EBackgroundColor::WHITE));

    titlePanel->AddRenderer(40, 1, 100, 3, std::move(titleText));
    titlePanel->Redraw();

    // ===== 안내 패널 (상단 중앙) =====
    Panel* guidePanel = _Drawer->CreatePanel("Guide", 2, 6, 120, 5);
    guidePanel->SetBorder(true, ETextColor::WHITE);

    UpdateGuidePanel(guidePanel);

    // ===== 아이템 이미지 패널 (중앙 좌측) =====
    Panel* shopImagePanel = _Drawer->CreatePanel("ShopImage", 2, 11, 50, 23);
    shopImagePanel->SetBorder(true, ETextColor::WHITE);

    auto imageText = std::make_unique<TextRenderer>();
    std::string uiPath = DataManager::GetInstance()->GetResourcePath("UI");

    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLine("");
    imageText->AddLineWithColor("    [아이템 이미지]",
        MakeColorAttribute(ETextColor::CYAN, EBackgroundColor::BLACK));
    shopImagePanel->SetContentRenderer(std::move(imageText));
    shopImagePanel->Redraw();

    // ===== 상품 목록 패널 (중앙 우측) =====
    Panel* itemListPanel = _Drawer->CreatePanel("ItemList", 54, 11, 68, 23);
    itemListPanel->SetBorder(true, ETextColor::WHITE);

    UpdateItemListPanel(itemListPanel);

    // ===== 타워 패널 (우측) - Stage와 공통 =====
    Panel* towerPanel = _Drawer->CreatePanel("Tower", 122, 1, 30, 30);
    auto towerArt = std::make_unique<AsciiArtRenderer>();

    bool towerLoaded = towerArt->LoadFromFile(uiPath, "Tower.txt");

    if (towerLoaded)
    {
        towerArt->SetAlignment(ArtAlignment::CENTER);
        towerArt->SetColor(ETextColor::WHITE);
        towerPanel->SetContentRenderer(std::move(towerArt));
    }
    else
    {
        auto errorText = std::make_unique<TextRenderer>();
        errorText->AddLine("");
        errorText->AddLine("");
        errorText->AddLineWithColor("[ Tower.txt not found ]",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
        errorText->AddLine("");
        towerPanel->SetContentRenderer(std::move(errorText));
    }

    // Tower 화살표 렌더링
    StageManager* stageMgr = StageManager::GetInstance();
    const StageFloorData* floorInfo = stageMgr->GetCurrentFloorInfo();
    if (floorInfo)
    {
        UpdateTowerArrow(towerPanel, floorInfo->Floor);
    }

    // ===== 시스템 로그 패널 (하단 좌측) - Stage와 공통 =====
    Panel* systemPanel = _Drawer->CreatePanel("System", 2, 34, 100, 11);
    systemPanel->SetBorder(true, ETextColor::WHITE);

    UpdateSystemLog(systemPanel, _SystemLogs);

    // ===== 인벤토리 패널 (하단 우측) - Stage와 공통 =====
    Panel* inventoryPanel = _Drawer->CreatePanel("Inventory", 102, 34, 47, 11);
    inventoryPanel->SetBorder(true, ETextColor::WHITE);

    UpdateInventoryPanel(inventoryPanel);

    _Drawer->Render();
}

void ShopScene::Exit()
{
    SoundPlayer::GetInstance()->ResetBGM();
    _Drawer->RemoveAllPanels();
    _IsActive = false;
}

void ShopScene::Update()
{
    if (_IsActive)
    {
        _Drawer->Update();
        HandleInput();
    }
}

void ShopScene::Render()
{
    // UIDrawer::Update()에서 자동 렌더링
}

void ShopScene::UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages)
{
    if (!systemPanel) return;

    systemPanel->ClearRenderers();
    systemPanel->SetDirty();

    auto logText = std::make_unique<TextRenderer>();
    logText->AddLineWithColor("[ 시스템 로그 ]",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    int maxLines = 8;
    int messageSize = static_cast<int>(messages.size());

    int startIndex = (messageSize > maxLines) ? (messageSize - maxLines) : 0;
    int displayCount = (messageSize > maxLines) ? maxLines : messageSize;

    for (int i = startIndex; i < startIndex + displayCount; ++i)
    {
        if (messages[i].empty())
        {
            logText->AddLine("");
            continue;
        }

        WORD color;
        if (messages[i].find("[디버그]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK);
        else if (messages[i].find("[경고]") != std::string::npos || messages[i].find("[오류]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK);
        else if (messages[i].find("[성공]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK);
        else if (messages[i].find("[정보]") != std::string::npos || messages[i].find("[안내]") != std::string::npos)
            color = MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK);
        else
            color = MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK);

        logText->AddLineWithColor(messages[i], color);
    }

    systemPanel->AddRenderer(0, 0, 98, 9, std::move(logText));
    systemPanel->Redraw();
}

void ShopScene::UpdateInventoryPanel(Panel* inventoryPanel)
{
    if (!inventoryPanel) return;

    auto inventoryText = std::make_unique<TextRenderer>();

    Player* player = GameManager::GetInstance()->GetMainPlayer().get();

    if (!player)
    {
        inventoryText->AddLineWithColor("[ 인벤토리 ]",
            MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
        inventoryText->AddLineWithColor("플레이어 정보를 불러올 수 없습니다.",
            MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    // 골드 표시
    std::string goldInfo = "[ 소지금: " + std::to_string(player->GetGold()) + " G ]";
    inventoryText->AddLineWithColor(goldInfo,
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    Inventory* inventory = nullptr;
    if (!player->TryGetInventory(inventory) || !inventory)
    {
        inventoryText->AddLine("");
        inventoryText->AddLineWithColor("인벤토리가 비활성화되어 있습니다.",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

        inventoryPanel->ClearRenderers();
        inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
        inventoryPanel->Redraw();
        return;
    }

    int usedSlots = 0;
    const int maxSlots = 5;
    for (int i = 0; i < maxSlots; ++i)
    {
        if (inventory->GetItemAtSlot(i) != nullptr)
            usedSlots++;
    }

    std::string header = "[ 인벤토리 (" + std::to_string(usedSlots) + "/" + std::to_string(maxSlots) + ") ]";
    inventoryText->AddLineWithColor(header,
        MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));

    for (int i = 0; i < maxSlots; ++i)
    {
        IItem* item = inventory->GetItemAtSlot(i);
        if (item)
        {
            int amount = inventory->GetSlotAmount(i);
            std::string itemLine = std::to_string(i + 1) + ". " +
                item->GetName() + " x" + std::to_string(amount);

            // 판매 모드에서 선택된 슬롯 하이라이트
            if (!_IsBuyMode && i == _PlayerInventorySlot)
            {
                inventoryText->AddLineWithColor("> " + itemLine,
                    MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));
            }
            else
            {
                inventoryText->AddLineWithColor(itemLine,
                    MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));
            }
        }
        else
        {
            std::string emptyLine = std::to_string(i + 1) + ". [빈 슬롯]";

            if (!_IsBuyMode && i == _PlayerInventorySlot)
            {
                inventoryText->AddLineWithColor("> " + emptyLine,
                    MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
            }
            else
            {
                inventoryText->AddLineWithColor(emptyLine,
                    MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
            }
        }
    }

    inventoryPanel->ClearRenderers();
    inventoryPanel->AddRenderer(0, 0, 45, 9, std::move(inventoryText));
    inventoryPanel->Redraw();
}

void ShopScene::UpdateTowerArrow(Panel* towerPanel, int currentFloor)
{
    auto arrowRenderer = std::make_unique<TextRenderer>();

    const int towerHeight = 25;
    const int maxFloor = 10;
    const int topMargin = 6;
    const int bottomMargin = 0;
    const int usableHeight = towerHeight - topMargin - bottomMargin;

    int arrowLine = topMargin + ((maxFloor - currentFloor) * usableHeight / maxFloor);

    for (int i = 0; i < arrowLine; ++i)
    {
        arrowRenderer->AddLine("");
    }

    arrowRenderer->AddLineWithColor("*----►",
        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

    towerPanel->AddRenderer(0, 0, 5, towerHeight, std::move(arrowRenderer));
    towerPanel->Redraw();
}

void ShopScene::UpdateGuidePanel(Panel* guidePanel)
{
    if (!guidePanel) return;

    auto guideText = std::make_unique<TextRenderer>();

    std::string controls = "[↑/↓] 선택   [Enter] 구매/판매   [Tab] 모드 전환   [ESC] 스테이지로 복귀";
    guideText->AddLineWithColor(controls,
        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK));

    std::string info = "구매 모드: 상점 아이템 선택 | 판매 모드: 인벤토리 아이템 선택 (판매가 = 구매가의 50%)";
    guideText->AddLineWithColor(info,
        MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));

    guidePanel->ClearRenderers();
    guidePanel->AddRenderer(2, 0, 116, 4, std::move(guideText));
    guidePanel->Redraw();
}

void ShopScene::UpdateItemListPanel(Panel* itemListPanel)
{
    if (!itemListPanel) return;

    auto itemText = std::make_unique<TextRenderer>();

    // 모드 표시
    if (_IsBuyMode)
    {
        itemText->AddLineWithColor("[ 구매 모드 ]",
            MakeColorAttribute(ETextColor::LIGHT_GREEN, EBackgroundColor::BLACK));
    }
    else
    {
        itemText->AddLineWithColor("[ 판매 모드 ]",
            MakeColorAttribute(ETextColor::LIGHT_MAGENTA, EBackgroundColor::BLACK));
    }

    itemText->AddLine("");

    if (_IsBuyMode)
    {
        // 구매 모드: 상점 아이템 목록 표시
        auto shopItems = ShopManager::GetInstance()->GetShopItems();

        if (shopItems.empty())
        {
            itemText->AddLineWithColor("판매 중인 상품이 없습니다.",
                MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
        }
        else
        {
            for (size_t i = 0; i < shopItems.size(); ++i)
            {
                std::string itemLine = std::to_string(i + 1) + ". " + shopItems[i].name +
                    " - " + std::to_string(shopItems[i].price) + " G";

                if (shopItems[i].stock > 0)
                {
                    itemLine += " (재고: " + std::to_string(shopItems[i].stock) + ")";
                }
                else
                {
                    itemLine += " [품절]";
                }

                if (i == static_cast<size_t>(_SelectedItemIndex))
                {
                    itemText->AddLineWithColor("> " + itemLine,
                        MakeColorAttribute(ETextColor::LIGHT_YELLOW, EBackgroundColor::BLACK));

                    // 선택된 아이템의 이미지 업데이트
                    UpdateItemImage(static_cast<int>(i));
                }
                else
                {
                    WORD color = (shopItems[i].stock > 0) ?
                        MakeColorAttribute(ETextColor::WHITE, EBackgroundColor::BLACK) :
                        MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK);
                    itemText->AddLineWithColor(itemLine, color);
                }
            }
        }
    }
    else
    {
        // 판매 모드: 인벤토리 아이템 표시 안내
        itemText->AddLineWithColor("우측 인벤토리에서",
            MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));
        itemText->AddLineWithColor("판매할 아이템을 선택하세요.",
            MakeColorAttribute(ETextColor::LIGHT_CYAN, EBackgroundColor::BLACK));
        itemText->AddLine("");
        itemText->AddLineWithColor("판매 가격은 구매 가격의 60%입니다.",
            MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
    }

    itemListPanel->ClearRenderers();
    itemListPanel->AddRenderer(0, 0, 64, 23, std::move(itemText));
    itemListPanel->Redraw();
}

void ShopScene::UpdateItemImage(int itemIndex)
{
    Panel* imagePanel = _Drawer->GetPanel("ShopImage");
    if (!imagePanel) return;

    std::string asciiFile = ShopManager::GetInstance()->GetItemAsciiFile(itemIndex);

    if (asciiFile.empty())
    {
        // 이미지 파일이 없으면 기본 텍스트 표시
        auto defaultText = std::make_unique<TextRenderer>();
        defaultText->AddLine("");
        defaultText->AddLine("");
        defaultText->AddLine("");
        defaultText->AddLine("");
        defaultText->AddLineWithColor("    [아이템 이미지]",
            MakeColorAttribute(ETextColor::CYAN, EBackgroundColor::BLACK));
        imagePanel->SetContentRenderer(std::move(defaultText));
    }
    else
    {
        // 아스키 아트 로드 시도
        std::string uiPath = DataManager::GetInstance()->GetItemsPath();
        auto artRenderer = std::make_unique<AsciiArtRenderer>();

        if (artRenderer->LoadFromFile(uiPath, asciiFile))
        {
            artRenderer->SetAlignment(ArtAlignment::CENTER);
            artRenderer->SetColor(ETextColor::WHITE);
            imagePanel->SetContentRenderer(std::move(artRenderer));
        }
        else
        {
            // 로드 실패 시 기본 텍스트
            auto errorText = std::make_unique<TextRenderer>();
            errorText->AddLine("");
            errorText->AddLine("");
            errorText->AddLine("");
            errorText->AddLineWithColor("    [이미지 로드 실패]",
                MakeColorAttribute(ETextColor::LIGHT_RED, EBackgroundColor::BLACK));
            errorText->AddLine("");
            errorText->AddLineWithColor("    " + asciiFile,
                MakeColorAttribute(ETextColor::DARK_GRAY, EBackgroundColor::BLACK));
            imagePanel->SetContentRenderer(std::move(errorText));
        }
    }

    imagePanel->Redraw();
}

void ShopScene::HandleInput()
{
    InputManager* input = InputManager::GetInstance();
    if (!input->IsKeyPressed()) return;

    int keyCode = input->GetKeyCode();

    // ===== ESC: 상점 나가기 =====
    if (keyCode == VK_ESCAPE)
    {
        _IsActive = false;
        Exit();
        SceneManager::GetInstance()->ChangeScene(ESceneType::StageSelect);
        return;
    }

    // ===== Tab: 구매/판매 모드 전환 =====
    if (keyCode == VK_TAB)
    {
        _IsBuyMode = !_IsBuyMode;
        _SelectedItemIndex = 0;
        _PlayerInventorySlot = 0;

        Panel* itemListPanel = _Drawer->GetPanel("ItemList");
        Panel* inventoryPanel = _Drawer->GetPanel("Inventory");

        UpdateItemListPanel(itemListPanel);
        UpdateInventoryPanel(inventoryPanel);

        if (_IsBuyMode)
    {
       _SystemLogs.push_back("[안내] 구매 모드로 전환되었습니다.");
        }
        else
        {
  _SystemLogs.push_back("[안내] 판매 모드로 전환되었습니다.");
        }

     Panel* systemPanel = _Drawer->GetPanel("System");
    UpdateSystemLog(systemPanel, _SystemLogs);

  _Drawer->Render();
        return;
    }

    // ===== 방향키: 위 =====
    if (keyCode == 72)  // VK_UP
    {
        if (_IsBuyMode)
        {
            _SelectedItemIndex--;
            if (_SelectedItemIndex < 0)
            {
                _SelectedItemIndex = static_cast<int>(ShopManager::GetInstance()->GetSellListSize()) - 1;
            }

            Panel* itemListPanel = _Drawer->GetPanel("ItemList");
            UpdateItemListPanel(itemListPanel);
        }
        else
        {
            _PlayerInventorySlot--;
            if (_PlayerInventorySlot < 0)
            {
                _PlayerInventorySlot = 4;  // 인벤토리 최대 슬롯 (5개: 0~4)
            }

            Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
            UpdateInventoryPanel(inventoryPanel);
        }

        _Drawer->Render();
        return;
    }

    // ===== 방향키: 아래 =====
    if (keyCode == 80)  // VK_DOWN
    {
        if (_IsBuyMode)
        {
            _SelectedItemIndex++;
            if (_SelectedItemIndex >= static_cast<int>(ShopManager::GetInstance()->GetSellListSize()))
            {
                _SelectedItemIndex = 0;
            }

            Panel* itemListPanel = _Drawer->GetPanel("ItemList");
            UpdateItemListPanel(itemListPanel);
        }
        else
        {
            _PlayerInventorySlot++;
            if (_PlayerInventorySlot > 4)
            {
                _PlayerInventorySlot = 0;
            }

            Panel* inventoryPanel = _Drawer->GetPanel("Inventory");
            UpdateInventoryPanel(inventoryPanel);
        }

        _Drawer->Render();
        return;
    }

    // ===== Enter: 구매/판매 실행 =====
    if (keyCode == VK_RETURN)
    {
        auto player = GameManager::GetInstance()->GetMainPlayer();
        if (!player)
        {
      _SystemLogs.push_back("[오류] 플레이어 정보를 불러올 수 없습니다.");
          Panel* systemPanel = _Drawer->GetPanel("System");
    UpdateSystemLog(systemPanel, _SystemLogs);
   return;
        }

  if (_IsBuyMode)
    {
     auto [success, message, goldChange, itemName] =
          ShopManager::GetInstance()->BuyItem(player.get(), _SelectedItemIndex);

            if (success)
{
           _SystemLogs.push_back("[성공] 구매 완료: " + itemName);
                _SystemLogs.push_back("[정보] 소지금 변동: " + std::to_string(goldChange) + " G");
            }
    else
        {
_SystemLogs.push_back("[경고] 구매 실패: " + message);
       }

    Panel* itemListPanel = _Drawer->GetPanel("ItemList");
   UpdateItemListPanel(itemListPanel);
        }
else
   {
     auto [success, message, goldChange, itemName] =
        ShopManager::GetInstance()->SellItem(player.get(), _PlayerInventorySlot);

if (success)
   {
        _SystemLogs.push_back("[성공] 판매 완료: " + itemName);
            _SystemLogs.push_back("[정보] 소지금 변동: +" + std::to_string(goldChange) + " G");
   }
          else
      {
      _SystemLogs.push_back("[경고] 판매 실패: " + message);
      }
        }

     Panel* systemPanel = _Drawer->GetPanel("System");
        Panel* inventoryPanel = _Drawer->GetPanel("Inventory");

  UpdateSystemLog(systemPanel, _SystemLogs);
   UpdateInventoryPanel(inventoryPanel);

   _Drawer->Render();
      return;
    }
}
