#pragma once
#include "../UIScene.h"
#include <vector>
#include <string>

class Panel;

// 상점 Scene
class ShopScene : public UIScene
{
private:
    int _SelectedItemIndex;  // 선택한 상품 인덱스
    bool _IsBuyMode;        // true: 구매 모드, false: 판매 모드
    int _PlayerInventorySlot;  // 판매 시 선택한 인벤토리 슬롯

    std::vector<std::string> _SystemLogs; // 로그 메시지를 누적합니다.

    // 공통 패널 업데이트 함수 (Stage와 공유)
    void UpdateSystemLog(Panel* systemPanel, const std::vector<std::string>& messages);
    void UpdateInventoryPanel(Panel* inventoryPanel);
    void UpdateTowerArrow(Panel* towerPanel, int currentFloor);
    
    // Shop 고유 패널 업데이트 함수
    void UpdateItemListPanel(Panel* itemListPanel);
    void UpdateGuidePanel(Panel* guidePanel);
    void UpdateItemImage(int itemIndex);  // 선택된 아이템 이미지 업데이트

public:
    ShopScene();
    ~ShopScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
};
