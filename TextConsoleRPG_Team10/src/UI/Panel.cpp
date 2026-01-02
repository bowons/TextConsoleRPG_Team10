#include "../../include/UI/Panel.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/IContentRenderer.h"

Panel::Panel(const std::string& id, const PanelBounds& bounds)
    : _PanelID(id), _Bounds(bounds)
{
}

Panel::~Panel()
{
}

void Panel::SetContentRenderer(std::unique_ptr<IContentRenderer> renderer)
{
    _ContentRenderer = std::move(renderer);
    SetDirty();
}

void Panel::RenderToBuffer(ScreenBuffer& buffer)
{
    // Dirty가 아니면 스킵 (최적화)
    if (!_IsDirty) {
        return;
    }

    // 패널 영역 클리어 (배경)
    buffer.FillRect(_Bounds.X, _Bounds.Y, _Bounds.Width, _Bounds.Height, ' ', 7);

    // 테두리 그리기
    if (_HasBorder) {
        buffer.DrawBox(_Bounds.X, _Bounds.Y, _Bounds.Width, _Bounds.Height, _BorderColor);
    }

    // 콘텐츠 렌더링
    if (_ContentRenderer) {
        _ContentRenderer->Render(buffer, _Bounds);
    }

    _IsDirty = false;
}

void Panel::SetBorder(bool enable, WORD color)
{
    _HasBorder = enable;
    _BorderColor = color;
    SetDirty();
}
