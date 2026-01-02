#pragma once
#include "Factory.h"
#include <string>
#include <memory>

class IItem;

// 아이템 전용 팩토리 클래스
// Factory<IItem> 템플릿을 상속받아 아이템 전용 기능 추가
class ItemFactory : public Factory<IItem>
{
public:
    ItemFactory();
    ~ItemFactory() = default;

private:
    // 기본 아이템 타입들을 팩토리에 등록
    void RegisterDefaultItems();
};