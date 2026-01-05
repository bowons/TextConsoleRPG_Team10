#include "../../include/Item/ShieldDefense.h"
#include "../../include/Unit/Player.h"

void ShieldDefense::ApplyEffect(Player& P)
{
    // 1라운드 동안 적용되는 방어력 버프
    P.ApplyTempDefBuff(_EffectAmount, 1);
}

std::unique_ptr<IItem> ShieldDefense::Clone() const
{
    return std::make_unique<ShieldDefense>();
}

bool ShieldDefense::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
        return false;
    }
    
    // HP 50% 이하일 때만 사용 가능
    return player.GetCurrentHP() <= player.GetMaxHP() * 0.5;
}

std::string ShieldDefense::GetUseConditionDescription() const
{
    return "HP 50% 이하";
}
