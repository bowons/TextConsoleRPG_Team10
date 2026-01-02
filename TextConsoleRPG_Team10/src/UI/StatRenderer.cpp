#include "../../include/UI/StatRenderer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"

void StatRenderer::SetStat(const std::string& key, const std::string& value)
{
    _Stats[key] = value;
    _IsDirty = true;
}

void StatRenderer::SetStat(const std::string& key, int value)
{
    _Stats[key] = std::to_string(value);
    _IsDirty = true;
}

void StatRenderer::RemoveStat(const std::string& key)
{
    _Stats.erase(key);
    _IsDirty = true;
}

void StatRenderer::Clear()
{
    _Stats.clear();
    _IsDirty = true;
}

void StatRenderer::Render(ScreenBuffer& buffer, const PanelBounds& bounds)
{
    if (!_IsDirty) return;

    // 패널 내부 영역
    int contentX = bounds.X + 1;
    int contentY = bounds.Y + 1;
    int contentWidth = bounds.Width - 2;
    int contentHeight = bounds.Height - 2;

    if (contentWidth <= 0 || contentHeight <= 0) return;

    int currentY = contentY;
    for (const auto& pair : _Stats) {
        if (currentY >= contentY + contentHeight) break;

 // "Key: Value" 형식
        std::string line = pair.first + ": " + pair.second;

        // 키 부분 색상
 buffer.WriteString(contentX, currentY, pair.first + ": ", _KeyColor);
     
        // 값 부분 색상
        int valueX = contentX + static_cast<int>(pair.first.length()) + 2;
   buffer.WriteString(valueX, currentY, pair.second, _ValueColor);

     currentY++;
    }

    _IsDirty = false;
}
