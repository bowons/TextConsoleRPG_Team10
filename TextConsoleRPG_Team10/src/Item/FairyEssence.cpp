#include "../../include/Item/FairyEssence.h"
#include "../../include/Unit/Player.h"

void FairyEssence::ApplyEffect(Player& P)
{
    P.ModifyMP(_EffectAmount);
}

std::unique_ptr<IItem> FairyEssence::Clone() const
{
    return std::make_unique<FairyEssence>();
}

bool FairyEssence::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
        return false;
    }
    
    // MP 40% 이하일 때만 사용 가능
    return player.GetCurrentMP() <= player.GetMaxMP() * 0.4;
}

std::string FairyEssence::GetUseConditionDescription() const
{
    return "MP 40% 이하";
}
