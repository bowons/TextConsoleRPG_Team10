#pragma once
#include <string>
#include <memory>
#include "../Config.h"

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
    virtual ~IItem() = default; //소멸자

    virtual void ApplyEffect(Player& Player) = 0; //아이템 사용 시 효과 적용
    
    // Clone 메서드: 동일한 타입의 새로운 아이템 인스턴스 생성
    // 인벤토리에서 여러 슬롯에 분산 저장 시 각 슬롯이 독립적인 아이템 객체를 소유하도록 함
    virtual std::unique_ptr<IItem> Clone() const = 0;
    
    int GetMaxCount() const { return _MaxCount; } //아이템 최대 스택 수 반환
    string GetName() const { return _ItemName; } //아이템 이름 반환
    int GetPrice() const { return _Price; } //아이템 가격 반환
};