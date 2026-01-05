#include "../../include/Item/TitanAwakening.h"
#include "../../include/Unit/Player.h"

void TitanAwakening::ApplyEffect(Player& P)
{
    // 1라운드 동안 적용되는 공격력 + 방어력 버프
    P.ApplyTempAtkBuff(_EffectAmount, 1);
    P.ApplyTempDefBuff(_EffectAmount, 1);
}

std::unique_ptr<IItem> TitanAwakening::Clone() const
{
    return std::make_unique<TitanAwakening>();
}

bool TitanAwakening::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
  return false;
    }
    
  // HP 50% 이하일 때 자동 사용
    return player.GetCurrentHP() <= player.GetMaxHP() * 0.5;
}

std::string TitanAwakening::GetUseConditionDescription() const
{
    return "HP 50% 이하 (자동)";
}
