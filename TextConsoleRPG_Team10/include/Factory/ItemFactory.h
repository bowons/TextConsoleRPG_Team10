#pragma once
#include "Factory.h"
#include <string>
#include <memory>

class IItem;
struct ItemData;

// 아이템 전용 팩토리 클래스
// Factory<IItem> 템플릿을 상속받아 아이템 전용 기능 추가
class ItemFactory : public Factory<IItem>
{
public:
    ItemFactory();
    ~ItemFactory() = default;

    // CSV 데이터를 기반으로 아이템 생성
    // itemData: CSV에서 읽은 아이템 데이터
    // return: 생성된 아이템 (실패 시 nullptr)
    std::unique_ptr<IItem> CreateFromData(const ItemData& itemData);

private:
    // 기본 아이템 타입들을 팩토리에 등록
    void RegisterDefaultItems();
};