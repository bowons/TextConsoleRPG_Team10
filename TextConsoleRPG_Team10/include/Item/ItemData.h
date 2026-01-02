#pragma once
#include <string>

// CSV 파일에서 아이템 데이터를 읽어올 때 사용하는 구조체
struct ItemData
{
    std::string ItemType;      // 아이템 타입 (예: "HealPotion")
    std::string Name;          // 아이템 이름 (현재는 사용 안 하지만 확장성 고려)
    int Price;                 // 구매 가격
    int EffectAmount;          // 효과 수치 (현재는 사용 안 하지만 확장성 고려)
    int MaxCount;              // 최대 스택 수 (현재는 사용 안 하지만 확장성 고려)
    int Stock;                 // 상점 초기 재고
};