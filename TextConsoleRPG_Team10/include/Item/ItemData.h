#pragma once
#include <string>

// CSV 파일에서 아이템 데이터를 읽어올 때 사용하는 구조체
struct ItemData
{
    std::string ItemID;        // 아이템 ID (예: "HealPotion", "AttackUp")
    std::string Name;          // 아이템 표시 이름
    std::string Description;   // 아이템 설명 (효과 포함)
    int Price = 0;             // 구매 가격
    int EffectAmount = 0;      // 효과 수치
    int MaxCount = 1;          // 최대 스택 수
    int Stock = 0;             // 상점 초기 재고
    std::string AsciiFile;     // 아스키 아트 파일 경로 (예: "Items/Potion.txt")
};