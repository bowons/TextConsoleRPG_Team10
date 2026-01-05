#include "../../include/Item/LuckyCharm.h"
#include "../../include/Unit/Player.h"
#include <random>

void LuckyCharm::ApplyEffect(Player& P)
{
    // 1라운드 동안 적용되는 운 버프
    P.ApplyTempLukBuff(_EffectAmount, 1);
}

std::unique_ptr<IItem> LuckyCharm::Clone() const
{
    return std::make_unique<LuckyCharm>();
}

bool LuckyCharm::CanUse(const Player& player, int currentRound) const
{
    // 예약되지 않았으면 사용 불가
    if (!IsReserved()) {
   return false;
    }
    
    // 전투 중 랜덤 확률 (30% 확률로 사용 가능)
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < 0.3;  // 30% 확률
}

std::string LuckyCharm::GetUseConditionDescription() const
{
    return "전투 중 랜덤 확률 (30%)";
}
