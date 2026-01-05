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

// ===== 기존 API (호환성) =====

void Panel::SetContentRenderer(std::unique_ptr<IContentRenderer> renderer)
{
    // 기존 모든 렌더러 제거 후 전체 영역으로 추가
    _RendererRegions.clear();
    
    if (renderer) {
        // 패널 전체 영역 (테두리 무시 - AddRenderer처럼 정확한 제어)
        // X, Y 오프셋 없음 (0, 0부터 시작)
        // Width, Height도 테두리 제외 없음 (패널 전체 크기)
        int contentX = 0;
        int contentY = 0;
        int contentWidth = _Bounds.Width;
        int contentHeight = _Bounds.Height;
        
        AddRenderer(contentX, contentY, contentWidth, contentHeight, std::move(renderer));
    }
    
    SetDirty();
}

IContentRenderer* Panel::GetContentRenderer()
{
    // 첫 번째 렌더러 반환 (기존 코드 호환)
    if (_RendererRegions.empty()) {
        return nullptr;
    }
    return _RendererRegions[0]->Renderer.get();
}

// ===== 영역 분할 API =====

void Panel::AddRenderer(int localX, int localY, int width, int height, 
          std::unique_ptr<IContentRenderer> renderer)
{
    if (!renderer) return;

    // 경계 클리핑 (패널 영역을 벗어나는 부분 자동 조정)
    if (localX < 0) { width += localX; localX = 0; }
    if (localY < 0) { height += localY; localY = 0; }
    if (localX + width > _Bounds.Width) width = _Bounds.Width - localX;
    if (localY + height > _Bounds.Height) height = _Bounds.Height - localY;
    
    if (width <= 0 || height <= 0) return;  // 유효하지 않은 영역

    PanelBounds localBounds(localX, localY, width, height);
    _RendererRegions.push_back(
        std::make_unique<RendererRegion>(localBounds, std::move(renderer))
    );
    SetDirty();
}

void Panel::ClearRenderers()
{
    _RendererRegions.clear();
    SetDirty();
}

void Panel::RemoveRenderer(size_t index)
{
    if (index < _RendererRegions.size()) {
        _RendererRegions.erase(_RendererRegions.begin() + index);
        SetDirty();
    }
}

void Panel::Update(float deltaTime)
{
    for (auto& region : _RendererRegions) {
        if (region && region->Renderer) {
            region->Renderer->Update(deltaTime);
            
            if (region->Renderer->IsDirty()) {
                SetDirty();
            }
        }
    }
}

void Panel::RenderToBuffer(ScreenBuffer& buffer)
{
    // 1. 패널 영역 클리어 (배경)
    buffer.FillRect(_Bounds.X, _Bounds.Y, _Bounds.Width, _Bounds.Height, ' ', 7);

    // 2. 테두리 그리기
    if (_HasBorder) {
        buffer.DrawBox(_Bounds.X, _Bounds.Y, _Bounds.Width, _Bounds.Height, _BorderColor);
    }

    // 3. 각 렌더러를 지정된 영역에 렌더링
    for (auto& region : _RendererRegions) {
        if (region && region->Renderer) {
            // 패널 내 상대 좌표를 절대 좌표로 변환
            PanelBounds absoluteBounds(
                _Bounds.X + region->LocalBounds.X,
                _Bounds.Y + region->LocalBounds.Y,
                region->LocalBounds.Width,
                region->LocalBounds.Height
            );
            region->Renderer->Render(buffer, absoluteBounds);
        }
    }
}

void Panel::SetBorder(bool enable, WORD color)
{
    _HasBorder = enable;
    _BorderColor = color;
    SetDirty();
}
