#pragma once
#include <string>

using namespace std;

class Player;

//아이템 인터페이스 클래스
class IItem
{
protected:
    string _ItemName; //아이템 이름
    int _Price; //아이템 가격
    int _EffectAmount; //아이템 효과 수치
    int _MaxCount; //아이템 최대 보유 가능한 개수

public:
    //생성자
    IItem(string ItemName, int Price, int EffectAmount, int MaxCount)
    : _ItemName(ItemName), _Price(Price), _EffectAmount(EffectAmount), _MaxCount(MaxCount){}
    
    virtual void ApplyEffect(Player& P) = 0; //아이템 사용 시 효과 적용
    virtual int GetMaxCount() const; //아이템 최대 스택 수 반환

    virtual ~IItem() = default; //소멸자
};