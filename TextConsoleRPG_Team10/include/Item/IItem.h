#pragma once
#include <string>

using namespace std;

class Player;

class IItem //아이템 인터페이스
{
protected:
    string _ItemName; //아이템 이름
    int _Price; //아이템 가격
    int _EffectAmount; //아이템 효과 수치
    int _MaxStack; //아이템 최대 개수 (= 스택 중첩 개념)

public:
    //생성자
    IItem(string ItemName, int Price, int EffectAmount, int MaxStack)
    : _ItemName(ItemName), _Price(Price), _EffectAmount(EffectAmount), _MaxStack(MaxStack){}
    
    virtual void ApplyEffect(Player& p) = 0; //아이템 사용 시 효과 적용
    virtual int GetMaxStack(); //아이템 최대 스택 수 반환
    virtual ~IItem() = default; //소멸자
};
