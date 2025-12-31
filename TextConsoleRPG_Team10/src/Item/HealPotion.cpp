#include "../../include/Item/HealPotion.h"
#include "../../include/Unit/Player.h"

void HealPotion::ApplyEffect(Player& p)
{
    //플레이어의 공격력 증가
    //※Heal 함수 Player 클래스에 있어야함 (동주님 코드 확인)
    p.Heal(_EffectAmount);
}
