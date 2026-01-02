#pragma once
#include "IContentRenderer.h"
#include <map>
#include <string>
#include <Windows.h>

// 스탯형 렌더러
// Key-Value 쌍으로 정보 표시 (플레이어 스탯, 몬스터 정보 등)
class StatRenderer : public IContentRenderer
{
private:
    std::map<std::string, std::string> _Stats;  // Key -> Value
    WORD _KeyColor = 11; // 키 색상 (LIGHT_CYAN)
    WORD _ValueColor = 14;  // 값 색상 (YELLOW)
    bool _IsDirty = true;

public:
    StatRenderer() = default;

    // 스탯 설정/업데이트
    void SetStat(const std::string& key, const std::string& value);
    void SetStat(const std::string& key, int value);
    void RemoveStat(const std::string& key);
    void Clear();

    // 색상 설정
    void SetKeyColor(WORD color) { _KeyColor = color; _IsDirty = true; }
    void SetValueColor(WORD color) { _ValueColor = color; _IsDirty = true; }

    // IContentRenderer 구현
    void Render(ScreenBuffer& buffer, const PanelBounds& bounds) override;
    bool IsDirty() const override { return _IsDirty; }
    void SetDirty() override { _IsDirty = true; }
};
