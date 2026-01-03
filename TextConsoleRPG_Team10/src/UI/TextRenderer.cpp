#include "../../include/UI/TextRenderer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"
#include "../../include/Manager/PrintManager.h"
#include <algorithm>

// ===== 기본 기능 =====

void TextRenderer::AddLine(const std::string& line)
{
    if (_AutoWrap && !line.empty())
    {
        // 자동 줄바꿈 활성화 시
        auto wrappedLines = WrapText(line, _WrapWidth);
        for (const auto& wrapped : wrappedLines)
        {
            _Lines.push_back(TextLine(wrapped, _DefaultTextColor));
        }
    }
    else
    {
        _Lines.push_back(TextLine(line, _DefaultTextColor));
    }
    _IsDirty = true;
}

void TextRenderer::AddLines(const std::vector<std::string>& lines)
{
    for (const auto& line : lines)
    {
        AddLine(line);
    }
}

void TextRenderer::Clear()
{
    _Lines.clear();
    _ScrollOffset = 0;
    _CurrentTypingLine = -1;
    _CurrentTypingChar = 0;
    _ColoredLineBuffer.clear();
    _IsDirty = true;
}

// ===== PrintManager 호환 기능 =====

void TextRenderer::AddLineWithColor(const std::string& line, WORD color)
{
    if (_AutoWrap && !line.empty())
    {
        auto wrappedLines = WrapText(line, _WrapWidth);
        for (const auto& wrapped : wrappedLines)
        {
            _Lines.push_back(TextLine(wrapped, color));
        }
    }
    else
    {
        _Lines.push_back(TextLine(line, color));
    }
    _IsDirty = true;
}

void TextRenderer::AddLogLine(const std::string& line, ELogImportance importance)
{
    std::string prefix;
    WORD color = _DefaultTextColor;

    switch (importance)
    {
    case ELogImportance::DISPLAY:
        prefix = "[DISPLAY] ";
        color = static_cast<WORD>(ETextColor::LIGHT_YELLOW);
        break;
    case ELogImportance::WARNING:
        prefix = "[WARNING] ";
        color = static_cast<WORD>(ETextColor::LIGHT_RED);
        break;
    default:
        break;
    }

    std::string fullLine = prefix + line;

    if (_AutoWrap && !fullLine.empty())
    {
        auto wrappedLines = WrapText(fullLine, _WrapWidth);
        for (size_t i = 0; i < wrappedLines.size(); ++i)
        {
            // 첫 줄만 prefix 색상, 나머지는 기본 색상
            WORD lineColor = (i == 0) ? color : _DefaultTextColor;
            _Lines.push_back(TextLine(wrappedLines[i], lineColor, importance));
        }
    }
    else
    {
        _Lines.push_back(TextLine(fullLine, color, importance));
    }

    _IsDirty = true;
}

void TextRenderer::AddLineWithTyping(const std::string& line, WORD color)
{
    TextLine textLine(line, color);
    textLine.HasTypingEffect = true;
    _Lines.push_back(textLine);

    if (_TypingEnabled && _CurrentTypingLine < 0)
    {
        // 타이핑 효과 시작
        _CurrentTypingLine = static_cast<int>(_Lines.size()) - 1;
        _CurrentTypingChar = 0;
        _LastTypingTime = std::chrono::steady_clock::now();
    }

    _IsDirty = true;
}

void TextRenderer::AddColoredText(const std::string& text, WORD color)
{
    // 임시 버퍼에 추가 (FlushColoredLine 호출 시 줄로 추가됨)
    _ColoredLineBuffer += text;
    _ColoredLineColor = color;  // 마지막 색상 저장
}

void TextRenderer::FlushColoredLine()
{
    if (!_ColoredLineBuffer.empty())
    {
        AddLineWithColor(_ColoredLineBuffer, _ColoredLineColor);
        _ColoredLineBuffer.clear();
    }
}

// ===== 타이핑 효과 =====

int TextRenderer::GetTypingInterval() const
{
    switch (_TypingSpeed)
    {
    case ETypingSpeed::Slow:
        return 100;
    case ETypingSpeed::Normal:
        return 50;
    case ETypingSpeed::Fast:
        return 20;
    default:
        return 50;
    }
}

bool TextRenderer::UpdateTypingEffect()
{
    if (!_TypingEnabled || _CurrentTypingLine < 0 || _CurrentTypingLine >= static_cast<int>(_Lines.size()))
        return false;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - _LastTypingTime).count();

    int interval = GetTypingInterval();

    if (elapsed >= interval)
    {
        TextLine& line = _Lines[_CurrentTypingLine];

        if (_CurrentTypingChar < line.Text.length())
        {
            // UTF-8 문자 처리
            unsigned char ch = static_cast<unsigned char>(line.Text[_CurrentTypingChar]);
            int charLen = 1;

            if (ch >= 0x80)
            {
                if ((ch & 0xE0) == 0xE0) charLen = 3;  // 한글
                else if ((ch & 0xC0) == 0xC0) charLen = 2;
            }

            _CurrentTypingChar += charLen;
            _LastTypingTime = now;
            _IsDirty = true;
            return true;
        }
        else
        {
            // 현재 줄 타이핑 완료
            _CurrentTypingLine = -1;
            _CurrentTypingChar = 0;

            // 다음 타이핑 효과 줄 찾기
            for (size_t i = _CurrentTypingLine + 1; i < _Lines.size(); ++i)
            {
                if (_Lines[i].HasTypingEffect)
                {
                    _CurrentTypingLine = static_cast<int>(i);
                    _CurrentTypingChar = 0;
                    _LastTypingTime = now;
                    break;
                }
            }
        }
    }

    return false;
}

void TextRenderer::Update(float deltaTime)
{
    if (UpdateTypingEffect())
    {
        _IsDirty = true;
    }
}

// ===== 자동 줄바꿈 =====

std::vector<std::string> TextRenderer::WrapText(const std::string& text, int maxWidth)
{
    std::vector<std::string> result;

    if (text.empty())
    {
        result.push_back("");
        return result;
    }

    std::string currentLine;
    int currentWidth = 0;

    for (size_t i = 0; i < text.length(); )
    {
        unsigned char ch = static_cast<unsigned char>(text[i]);
        int charLen = 1;
        int visualWidth = 1;

        // UTF-8 문자 크기 계산
        if (ch >= 0x80)
        {
            if ((ch & 0xE0) == 0xE0) charLen = 3;  // 한글
            else if ((ch & 0xC0) == 0xC0) charLen = 2;
            visualWidth = 2;  // 한글은 2칸
        }

        // 줄바꿈 필요 여부 확인
        if (currentWidth + visualWidth > maxWidth && !currentLine.empty())
        {
            result.push_back(currentLine);
            currentLine.clear();
            currentWidth = 0;
        }

        // 문자 추가
        for (int j = 0; j < charLen && (i + j) < text.length(); ++j)
        {
            currentLine += text[i + j];
        }

        currentWidth += visualWidth;
        i += charLen;
    }

    // 마지막 줄 추가
    if (!currentLine.empty())
    {
        result.push_back(currentLine);
    }

    return result;
}

// ===== 렌더링 =====

void TextRenderer::Render(ScreenBuffer& buffer, const PanelBounds& bounds)
{
    // 패널 내부 영역 계산 (테두리 제외)
    int contentX = bounds.X + 1;
    int contentY = bounds.Y + 1;
    int contentWidth = bounds.Width - 2;
    int contentHeight = bounds.Height - 2;

    if (contentWidth <= 0 || contentHeight <= 0) {
        _IsDirty = false;
        return;
    }

    // 자동 스크롤: 항상 최신 줄 표시
    if (_AutoScroll && _Lines.size() > static_cast<size_t>(contentHeight))
    {
        _ScrollOffset = static_cast<int>(_Lines.size()) - contentHeight;
    }

    // 렌더링할 줄 선택
    int startLine = _ScrollOffset;
    int endLine = (std::min)(startLine + contentHeight, static_cast<int>(_Lines.size()));

    int currentY = contentY;
    for (int i = startLine; i < endLine; ++i)
    {
        if (currentY >= contentY + contentHeight) break;

        const TextLine& line = _Lines[i];

        // 타이핑 효과 처리
        std::string displayText = line.Text;
        if (_TypingEnabled && line.HasTypingEffect && i == _CurrentTypingLine)
        {
            // 현재 타이핑 중인 줄은 일부만 표시
            displayText = line.Text.substr(0, _CurrentTypingChar);
        }

        // 텍스트 쓰기 (패널 너비 초과 시 자름)
        buffer.WriteString(contentX, currentY, displayText, line.Color);

        currentY++;
    }

    _IsDirty = false;
}
