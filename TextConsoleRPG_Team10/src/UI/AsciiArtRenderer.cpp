#include "../../include/UI/AsciiArtRenderer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include <sstream>
#include <algorithm>

bool AsciiArtRenderer::LoadFromFile(const std::string& folderPath, const std::string& fileName)
{
  std::string content = DataManager::GetInstance()->LoadTextFile(folderPath, fileName);
    if (content.empty()) {
     PrintManager::GetInstance()->PrintLogLine(
            "AsciiArtRenderer: Failed to load " + fileName, ELogImportance::WARNING);
    return false;
    }

    return LoadFromString(content);
}

bool AsciiArtRenderer::LoadFromString(const std::string& artContent)
{
    _ArtLines.clear();
    
    std::istringstream stream(artContent);
    std::string line;
    while (std::getline(stream, line)) {
        // \r 제거
   if (!line.empty() && line.back() == '\r') {
line.pop_back();
        }
    _ArtLines.push_back(line);
    }

 _IsDirty = true;
    return !_ArtLines.empty();
}

bool AsciiArtRenderer::LoadAnimationFromFiles(const std::string& folderPath,
    const std::vector<std::string>& fileNames, float frameDuration)
{
    _AnimationFrames.clear();

    for (const auto& fileName : fileNames) {
        std::string content = DataManager::GetInstance()->LoadTextFile(folderPath, fileName);
        if (content.empty()) {
            PrintManager::GetInstance()->PrintLogLine(
              "AsciiArtRenderer: Failed to load animation frame " + fileName,
ELogImportance::WARNING);
 continue;
        }

  std::vector<std::string> frame;
        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r') {
 line.pop_back();
            }
            frame.push_back(line);
   }

        _AnimationFrames.push_back(std::move(frame));
    }

  if (!_AnimationFrames.empty()) {
        _FrameDuration = frameDuration;
        _CurrentFrame = 0;
        _IsAnimating = true;
        _IsDirty = true;
        return true;
  }

    return false;
}

void AsciiArtRenderer::Clear()
{
    _ArtLines.clear();
    _AnimationFrames.clear();
    _IsDirty = true;
}

void AsciiArtRenderer::Update(float deltaTime)
{
    if (!_IsAnimating || _AnimationFrames.empty()) return;

    _FrameTime += deltaTime;
    if (_FrameTime >= _FrameDuration) {
     _FrameTime = 0.0f;
     _CurrentFrame = (_CurrentFrame + 1) % _AnimationFrames.size();
        _IsDirty = true;
    }
}

void AsciiArtRenderer::Render(ScreenBuffer& buffer, const PanelBounds& bounds)
{
    if (!_IsDirty) return;

    // 애니메이션 모드
    if (_IsAnimating && !_AnimationFrames.empty()) {
   RenderFrame(buffer, bounds, _AnimationFrames[_CurrentFrame]);
    }
    // 단일 아트 모드
    else {
        RenderFrame(buffer, bounds, _ArtLines);
    }

    _IsDirty = false;
}

void AsciiArtRenderer::RenderFrame(ScreenBuffer& buffer, const PanelBounds& bounds,
    const std::vector<std::string>& frame)
{
    if (frame.empty()) return;

    int contentX = bounds.X + 1;
    int contentY = bounds.Y + 1;
    int contentWidth = bounds.Width - 2;
    int contentHeight = bounds.Height - 2;

    if (contentWidth <= 0 || contentHeight <= 0) return;

    // 아트 크기 검증
    int artHeight = static_cast<int>(frame.size());
    int artWidth = 0;
  for (const auto& line : frame) {
        artWidth = (std::max)(artWidth, static_cast<int>(line.length()));
    }

    // 패널보다 크면 경고
    if (artHeight > contentHeight || artWidth > contentWidth) {
        PrintManager::GetInstance()->PrintLogLine(
        "AsciiArtRenderer: Art size exceeds panel bounds (will be clipped)",
     ELogImportance::WARNING);
    }

    // 정렬 계산
    int startX = contentX;
    int startY = contentY;

    switch (_Alignment) {
    case ArtAlignment::CENTER:
    startX = contentX + (contentWidth - artWidth) / 2;
        startY = contentY + (contentHeight - artHeight) / 2;
        break;
  case ArtAlignment::RIGHT:
        startX = contentX + contentWidth - artWidth;
     break;
    case ArtAlignment::LEFT:
    default:
     break;
    }

    // 렌더링
    int currentY = startY;
    for (const auto& line : frame) {
        if (currentY >= contentY + contentHeight) break;
        if (currentY >= bounds.Y + bounds.Height - 1) break;

        buffer.WriteString(startX, currentY, line, _Color);
        currentY++;
    }
}
