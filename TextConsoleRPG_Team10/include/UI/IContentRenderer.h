#pragma once

class ScreenBuffer;
struct PanelBounds;

// 콘텐츠 렌더러 인터페이스
// 각 패널에 다양한 형태의 콘텐츠를 표시하기 위한 추상 클래스
class IContentRenderer
{
public:
    virtual ~IContentRenderer() = default;

    // 패널 내부에 콘텐츠 렌더링
    // buffer: 화면 버퍼
    // bounds: 패널의 경계 (렌더링 가능한 영역)
    virtual void Render(ScreenBuffer& buffer, const PanelBounds& bounds) = 0;

    // 콘텐츠 업데이트 (애니메이션, 타이핑 효과 등)
    // deltaTime: 이전 프레임으로부터 경과 시간 (초)
    virtual void Update(float deltaTime) {}

    // Dirty 플래그 (재렌더링 필요 여부)
    virtual bool IsDirty() const { return true; }
    virtual void SetDirty() {}
};
