#pragma once
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <vector>

// 범용 팩토리 템플릿 클래스
// T: 생성할 객체의 기본 타입 (예: IItem, IMonster 등)
template <typename T>
class Factory
{
private:
    // 타입 이름 -> 생성 함수 매핑
    std::map<std::string, std::function<std::unique_ptr<T>()>> _FactoryMap;

public:
    Factory() = default;
    virtual ~Factory() = default;

    // 타입 이름으로 객체 인스턴스 생성
    std::unique_ptr<T> Create(const std::string& typeName) const
    {
        auto it = _FactoryMap.find(typeName);

        if (it != _FactoryMap.end())
        {
            return it->second();  // 등록된 팩토리 함수 호출
        }

        return nullptr;  // 등록되지 않은 타입
    }

    // 새로운 타입 등록
    void Register(const std::string& typeName, std::function<std::unique_ptr<T>()> factory)
    {
        _FactoryMap[typeName] = factory;
    }

    // 등록된 모든 타입 이름 반환
    std::vector<std::string> GetRegisteredTypes() const
    {
        std::vector<std::string> types;
        types.reserve(_FactoryMap.size());

        for (const auto& pair : _FactoryMap)
        {
            types.push_back(pair.first);
        }

        return types;
    }

    // 특정 타입이 등록되어 있는지 확인
    bool IsRegistered(const std::string& typeName) const
    {
        return _FactoryMap.find(typeName) != _FactoryMap.end();
    }

    // 등록된 타입 개수 반환
    size_t GetRegisteredCount() const
    {
        return _FactoryMap.size();
    }
};
