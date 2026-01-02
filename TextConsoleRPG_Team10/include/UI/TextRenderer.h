#pragma once
#include "IContentRenderer.h"
#include <vector>
#include <string>
#include <Windows.h>

// 기본 텍스트 렌더러
// 여러 줄의 텍스트를 표시 (스크롤 로그 등에 사용)
class TextRenderer : public IContentRenderer
{
private:
    std::vector<std::string> _Lines;
    WORD _TextColor = 7;  // 기본 색상
    bool _AutoScroll = true;  // 자동 스크롤 (최신 줄 표시)
    int _ScrollOffset = 0;    // 스크롤 위치
    bool _IsDirty = true;

public:
    TextRenderer() = default;

    // 텍스트 추가
    void AddLine(const std::string& line);
 void AddLines(const std::vector<std::string>& lines);

 // 텍스트 클리어
    void Clear();

    // 색상 설정
    void SetTextColor(WORD color) { _TextColor = color; _IsDirty = true; }

    // 스크롤 설정
    void SetAutoScroll(bool enable) { _AutoScroll = enable; }
    void ScrollUp() { if (_ScrollOffset > 0) _ScrollOffset--; }
    void ScrollDown() { _ScrollOffset++; }

    // IContentRenderer 구현
    void Render(ScreenBuffer& buffer, const PanelBounds& bounds) override;
    bool IsDirty() const override { return _IsDirty; }
 void SetDirty() override { _IsDirty = true; }
};
