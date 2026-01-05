#include "../../include/Item/AttackUp.h"
#include "../../include/Unit/Player.h"

void AttackUp::ApplyEffect(Player& P)
{
    // 1라운드 동안 적용되는 공격력 버프
    P.ApplyTempAtkBuff(_EffectAmount, 1);
}

std::unique_ptr<IItem> AttackUp::Clone() const
{
    // 새로운 AttackUp 인스턴스 생성하여 반환
    // 각 슬롯이 독립적인 아이템 객체를 소유하도록 함
    return std::make_unique<AttackUp>();
}

bool AttackUp::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
        return false;
    }
    
    // 전투 시작 후 1턴 경과 시 사용 가능
    int turnsSinceReserved = currentRound - GetReservedRound();
    return turnsSinceReserved >= 1;
}

std::string AttackUp::GetUseConditionDescription() const
{
    return "전투 시작 후 1턴 경과";
}