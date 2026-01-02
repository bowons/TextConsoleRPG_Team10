#include "../../include/Item/HealPotion.h"
#include "../../include/Unit/Player.h"

void HealPotion::ApplyEffect(Player& P)
{
    P.ModifyHP(_EffectAmount);
}

std::unique_ptr<IItem> HealPotion::Clone() const
{
    // 새로운 HealPotion 인스턴스 생성하여 반환
    // 각 슬롯이 독립적인 아이템 객체를 소유하도록 함
    return std::make_unique<HealPotion>();
}
