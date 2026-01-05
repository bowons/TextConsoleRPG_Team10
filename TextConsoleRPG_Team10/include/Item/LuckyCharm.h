#pragma once
#include "IItem.h"

// 플레이어 운 증가 아이템
class LuckyCharm : public IItem
{
public:
    // 생성자
    // 운 증가 아이템 설정 (이름, 금액, 증가 수치, 보유 가능 개수)
    LuckyCharm() : IItem("행운의 부적", 300, 10, 5) {};

    void ApplyEffect(Player& Player) override;

    // 동일한 타입의 새 LuckyCharm 인스턴스 생성
    std::unique_ptr<IItem> Clone() const override;
    
    // 아이템 사용 가능 여부 판단
    // player: 사용자
    // currentRound: 현재 라운드 (0부터 시작)
    // return: 사용 가능하면 true
    bool CanUse(const Player& player, int currentRound) const override;
    
 // 사용 조건 설명 반환 (UI 표시용)
    // return: 조건 설명 문자열
    std::string GetUseConditionDescription() const override;
};
