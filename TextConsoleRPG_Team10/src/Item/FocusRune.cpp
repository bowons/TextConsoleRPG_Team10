#include "../../include/Item/FocusRune.h"
#include "../../include/Unit/Player.h"

void FocusRune::ApplyEffect(Player& P)
{
    // 1라운드 동안 적용되는 민첩성 버프
    P.ApplyTempDexBuff(_EffectAmount, 1);
}

std::unique_ptr<IItem> FocusRune::Clone() const
{
    return std::make_unique<FocusRune>();
}

bool FocusRune::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
    return false;
    }
    
    // 전투 시작 시 언제든지 사용 가능
    return true;
}

std::string FocusRune::GetUseConditionDescription() const
{
    return "전투 시작 시 (언제든지)";
}
