#pragma once
#include <string>
#include <memory>
#include <vector>
#include <Windows.h>

class ScreenBuffer;
class IContentRenderer;

// 패널 경계
struct PanelBounds {
    int X = 0;
    int Y = 0;
    int Width = 10;
    int Height = 10;

    PanelBounds() = default;
    PanelBounds(int x, int y, int w, int h) : X(x), Y(y), Width(w), Height(h) {}

    bool Contains(int x, int y) const {
 return x >= X && x < X + Width && y >= Y && y < Y + Height;
    }
};

// 개별 UI 패널
class Panel
{
private:
    std::string _PanelID;
    PanelBounds _Bounds;
    std::unique_ptr<IContentRenderer> _ContentRenderer;
    bool _IsDirty = true;      // 재렌더링 필요 여부
    bool _HasBorder = false;   // 테두리 표시 여부
    WORD _BorderColor = 7;     // 테두리 색상

public:
    Panel(const std::string& id, const PanelBounds& bounds);
    ~Panel();

    // 콘텐츠 렌더러 설정
    void SetContentRenderer(std::unique_ptr<IContentRenderer> renderer);

    // 패널을 화면 버퍼에 렌더링
 void RenderToBuffer(ScreenBuffer& buffer);

    // 강제 재렌더링
    void Redraw() { _IsDirty = true; }

    // 테두리 설정
    void SetBorder(bool enable, WORD color = 7);

    // Dirty 플래그 관리
    inline void SetDirty() { _IsDirty = true; }
    inline bool IsDirty() const { return _IsDirty; }
    inline void ClearDirty() { _IsDirty = false; }

    // Getter
    inline const std::string& GetID() const { return _PanelID; }
    inline const PanelBounds& GetBounds() const { return _Bounds; }
    inline PanelBounds& GetBounds() { return _Bounds; }
    IContentRenderer* GetContentRenderer() { return _ContentRenderer.get(); }
};
