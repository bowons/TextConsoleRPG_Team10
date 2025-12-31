#include "../../include/Item/IItem.h"

int IItem::GetMaxStack() const
{
    //슬롯 당 최대 보유 개수 반환
    return _MaxStack;
}
