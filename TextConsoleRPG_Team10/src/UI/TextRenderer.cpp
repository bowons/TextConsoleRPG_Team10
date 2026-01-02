#include "../../include/UI/TextRenderer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"
#include <algorithm>  // std::min 사용

void TextRenderer::AddLine(const std::string& line)
{
    _Lines.push_back(line);
    _IsDirty = true;
}

void TextRenderer::AddLines(const std::vector<std::string>& lines)
{
    _Lines.insert(_Lines.end(), lines.begin(), lines.end());
    _IsDirty = true;
}

void TextRenderer::Clear()
{
    _Lines.clear();
    _ScrollOffset = 0;
    _IsDirty = true;
}

void TextRenderer::Render(ScreenBuffer& buffer, const PanelBounds& bounds)
{
    if (!_IsDirty) return;

    // 패널 내부 영역 계산 (테두리 제외)
    int contentX = bounds.X + 1;
    int contentY = bounds.Y + 1;
 int contentWidth = bounds.Width - 2;
    int contentHeight = bounds.Height - 2;

    if (contentWidth <= 0 || contentHeight <= 0) return;

    // 자동 스크롤: 항상 최신 줄 표시
    if (_AutoScroll && _Lines.size() > static_cast<size_t>(contentHeight)) {
  _ScrollOffset = static_cast<int>(_Lines.size()) - contentHeight;
    }

    // 렌더링할 줄 선택
    int startLine = _ScrollOffset;
    int endLine = (std::min)(startLine + contentHeight, static_cast<int>(_Lines.size()));

    int currentY = contentY;
    for (int i = startLine; i < endLine; ++i) {
        if (currentY >= contentY + contentHeight) break;

        const std::string& line = _Lines[i];
        
        // 텍스트 쓰기 (패널 너비 초과 시 자름)
   buffer.WriteString(contentX, currentY, line, _TextColor);
 
 currentY++;
 }

    _IsDirty = false;
}
