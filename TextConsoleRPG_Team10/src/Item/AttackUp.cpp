#include "../../include/Item/AttackUp.h"
#include "../../include/Unit/Player.h"

void AttackUp::ApplyEffect(Player& p)
{
    //플레이어의 공격력 증가
    //※AddAttack 함수 Player 클래스에 있어야함 (동주님 코드 확인)
    p.AddAttack(_EffectAmount);
}