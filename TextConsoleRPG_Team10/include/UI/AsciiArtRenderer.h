#pragma once
#include "IContentRenderer.h"
#include "../Common/TextColor.h"  // ETextColor 사용을 위해 추가
#include <vector>
#include <string>
#include <Windows.h>

enum class ArtAlignment {
    LEFT,
    CENTER,
    RIGHT
};

// 아스키 아트 렌더러
// 텍스트 파일에서 아스키 아트를 로드하여 표시
class AsciiArtRenderer : public IContentRenderer
{
private:
    std::vector<std::string> _ArtLines;  // 아트 내용 (줄 단위)
    WORD _Color = 7;
    ArtAlignment _Alignment = ArtAlignment::CENTER;
    bool _IsDirty = true;

    // 애니메이션용
    std::vector<std::vector<std::string>> _AnimationFrames;  // 여러 프레임
    int _CurrentFrame = 0;
    float _FrameTime = 0.0f;
    float _FrameDuration = 0.5f;  // 프레임당 시간 (초)
    bool _IsAnimating = false;

    // 에러 처리용
    bool _HasError = false;
    std::string _LastError;
    std::string _LastFilePath;  // 마지막 시도한 파일 경로

public:
    AsciiArtRenderer() = default;

    // 단일 아스키 아트 로드
    bool LoadFromFile(const std::string& folderPath, const std::string& fileName);
    bool LoadFromString(const std::string& artContent);

    // 애니메이션 로드 (여러 파일)
    bool LoadAnimationFromFiles(const std::string& folderPath,
        const std::vector<std::string>& fileNames,
        float frameDuration = 0.5f);

    // 폴더 내 모든 파일을 애니메이션으로 로드
    bool LoadAnimationFromFolder(const std::string& animationFolderPath,
        float frameDuration = 0.5f,
        const std::string& extension = ".txt");

    // JSON 파일에서 애니메이션 로드
    bool LoadAnimationFromJson(const std::string& folderPath, const std::string& fileName);

    // 아트 클리어
    void Clear();

    // 정렬 설정
    void SetAlignment(ArtAlignment alignment) { _Alignment = alignment; _IsDirty = true; }
    void SetColor(WORD color) { _Color = color; _IsDirty = true; }
    void SetColor(ETextColor color) { _Color = static_cast<WORD>(color); _IsDirty = true; }  // 오버로드 추가

    // 애니메이션 제어
    void StartAnimation() { _IsAnimating = true; }
    void StopAnimation() { _IsAnimating = false; }
    void SetFrameDuration(float duration) { _FrameDuration = duration; }

    // 에러 정보 접근
    bool HasError() const { return _HasError; }
    const std::string& GetLastError() const { return _LastError; }
    const std::string& GetLastFilePath() const { return _LastFilePath; }  // 추가
    
    // 에러 초기화 (재시도 전)
    void ClearError() { _HasError = false; _LastError.clear(); _LastFilePath.clear(); }

    // IContentRenderer 구현
    void Render(ScreenBuffer& buffer, const PanelBounds& bounds) override;
    void Update(float deltaTime) override;
    bool IsDirty() const override { return _IsDirty; }
    void SetDirty() override { _IsDirty = true; }

private:
    void RenderFrame(ScreenBuffer& buffer, const PanelBounds& bounds,
        const std::vector<std::string>& frame);
};
