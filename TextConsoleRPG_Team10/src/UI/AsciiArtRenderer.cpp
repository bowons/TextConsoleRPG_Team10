#include "../../include/UI/AsciiArtRenderer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include <sstream>
#include <algorithm>
#include <fstream>

// UTF-8 바이트 시퀀스를 유니코드 코드포인트로 변환
static int DecodeUTF8CodePoint(const std::string& str, size_t index, int& outCharLen)
{
    unsigned char ch = static_cast<unsigned char>(str[index]);
    outCharLen = 1;
    int codepoint = 0;

    if ((ch & 0x80) == 0) {  // 1바이트 (ASCII)
        outCharLen = 1;
        codepoint = ch;
    }
    else if ((ch & 0xE0) == 0xC0) {  // 2바이트
        outCharLen = 2;
        if (index + 1 < str.length()) {
            codepoint = ((ch & 0x1F) << 6) | (str[index + 1] & 0x3F);
        }
    }
    else if ((ch & 0xF0) == 0xE0) {  // 3바이트
        outCharLen = 3;
        if (index + 2 < str.length()) {
            codepoint = ((ch & 0x0F) << 12) |
                         ((str[index + 1] & 0x3F) << 6) |
                         (str[index + 2] & 0x3F);
        }
    }
    else if ((ch & 0xF8) == 0xF0) {  // 4바이트
        outCharLen = 4;
        if (index + 3 < str.length()) {
            codepoint = ((ch & 0x07) << 18) |
                         ((str[index + 1] & 0x3F) << 12) |
                         ((str[index + 2] & 0x3F) << 6) |
                         (str[index + 3] & 0x3F);
        }
    }

    return codepoint;
}

// 유니코드 코드포인트의 시각적 너비 계산
static int GetCodePointVisualWidth(int codepoint)
{
    // ASCII: 1칸
    if (codepoint < 0x80)
        return 1;

    // 점자 패턴 (U+2800 ~ U+28FF): 1칸
    if (codepoint >= 0x2800 && codepoint <= 0x28FF)
        return 1;

    // Box Drawing Characters (U+2500 ~ U+257F): 1칸
    if (codepoint >= 0x2500 && codepoint <= 0x257F)
        return 1;

    // Block Elements (U+2580 ~ U+259F): 1칸
    if (codepoint >= 0x2580 && codepoint <= 0x259F)
        return 1;

    // 한글 음절 (U+AC00 ~ U+D7A3): 2칸
    if (codepoint >= 0xAC00 && codepoint <= 0xD7A3)
        return 2;

    // 한글 자모: 2칸
    if ((codepoint >= 0x1100 && codepoint <= 0x11FF) ||
        (codepoint >= 0x3130 && codepoint <= 0x318F))
        return 2;

    // 한자 및 CJK: 2칸
    if ((codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
        (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
        (codepoint >= 0xF900 && codepoint <= 0xFAFF))
        return 2;

    // 기타: 1칸
    return 1;
}

bool AsciiArtRenderer::LoadFromFile(const std::string& folderPath, const std::string& fileName)
{
    _LastFilePath = folderPath + "/" + fileName;  // 경로 저장

    std::string content = DataManager::GetInstance()->LoadTextFile(folderPath, fileName);
    if (content.empty()) {
        // 에러 정보 저장
        _LastError = "Failed to load file: " + _LastFilePath;
        _HasError = true;
        return false;
    }

    _HasError = false;
    _LastError.clear();
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

    if (_ArtLines.empty()) {
        _LastError = "Art content is empty";
        _HasError = true;
        _IsDirty = true;
        return false;
    }

    _HasError = false;
    _LastError.clear();
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

bool AsciiArtRenderer::LoadAnimationFromFolder(const std::string& animationFolderPath,
    float frameDuration,
    const std::string& extension)
{
    // 1. 폴더 내 모든 파일 목록 가져오기
    std::vector<std::string> fileNames =
        DataManager::GetInstance()->GetFilesInDirectory(animationFolderPath, extension);

    if (fileNames.empty())
    {
        PrintManager::GetInstance()->PrintLogLine(
            "AsciiArtRenderer: No files found in folder " + animationFolderPath,
            ELogImportance::WARNING);
        return false;
    }

    // 2. 기존 LoadAnimationFromFiles 호출
    return LoadAnimationFromFiles(animationFolderPath, fileNames, frameDuration);
}

// JSON 파일에서 애니메이션 로드
bool AsciiArtRenderer::LoadAnimationFromJson(const std::string& folderPath, const std::string& fileName)
{
    _AnimationFrames.clear();
    _LastFilePath = folderPath + "/" + fileName;

    // JSON 파일 로드
    std::string jsonContent = DataManager::GetInstance()->LoadTextFile(folderPath, fileName);
    if (jsonContent.empty()) {
        _LastError = "Failed to load JSON file: " + _LastFilePath;
        _HasError = true;
        PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
        return false;
    }

    // "frames" 찾기
    size_t framesPos = jsonContent.find("\"frames\"");
    if (framesPos == std::string::npos) {
        _LastError = "JSON format error: 'frames' not found";
        _HasError = true;
        PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
        return false;
    }

    // frames 배열 시작 찾기
    size_t colonPos = jsonContent.find(':', framesPos);
    if (colonPos == std::string::npos) {
        _LastError = "JSON format error: colon after 'frames' not found";
        _HasError = true;
        PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
        return false;
    }

    size_t arrayStart = jsonContent.find('[', colonPos);
    if (arrayStart == std::string::npos) {
        _LastError = "JSON format error: frames array not found";
        _HasError = true;
        PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
        return false;
    }

    // 각 프레임 파싱 (단일 문자열 형태)
    size_t pos = arrayStart + 1;
    int frameCount = 0;

    while (pos < jsonContent.length()) {
        // 공백 건너뛰기
        while (pos < jsonContent.length() &&
            (jsonContent[pos] == ' ' || jsonContent[pos] == '\n' ||
                jsonContent[pos] == '\r' || jsonContent[pos] == '\t')) {
            pos++;
        }

        if (pos >= jsonContent.length()) break;

        // 배열 끝 확인
        if (jsonContent[pos] == ']') {
            break;
        }

        // 콤마 건너뛰기
        if (jsonContent[pos] == ',') {
            pos++;
            continue;
        }

        // 문자열 시작 " 찾기
        if (jsonContent[pos] != '"') {
            pos++;
            continue;
        }

        size_t stringStart = pos + 1;

        // 문자열 끝 " 찾기 (이스케이프 처리)
        pos++;
        while (pos < jsonContent.length()) {
            if (jsonContent[pos] == '\\' && pos + 1 < jsonContent.length()) {
                pos += 2; // 이스케이프 문자 건너뛰기
                continue;
            }
            if (jsonContent[pos] == '"') {
                break;
            }
            pos++;
        }

        if (pos >= jsonContent.length()) {
            _LastError = "JSON format error: unterminated string";
            _HasError = true;
            PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
            return false;
        }

        size_t stringEnd = pos;

        // 프레임 문자열 추출
        std::string frameString = jsonContent.substr(stringStart, stringEnd - stringStart);

        // 이스케이프 문자 처리
        std::string processedString;
        for (size_t i = 0; i < frameString.length(); ++i) {
            if (frameString[i] == '\\' && i + 1 < frameString.length()) {
                if (frameString[i + 1] == 'n') {
                    processedString += '\n';
                    i++;
                }
                else if (frameString[i + 1] == '\\') {
                    processedString += '\\';
                    i++;
                }
                else if (frameString[i + 1] == '"') {
                    processedString += '"';
                    i++;
                }
                else if (frameString[i + 1] == 't') {
                    processedString += '\t';
                    i++;
                }
                else if (frameString[i + 1] == 'r') {
                    processedString += '\r';
                    i++;
                }
                else {
                    processedString += frameString[i];
                }
            }
            else {
                processedString += frameString[i];
            }
        }

        // 문자열을 개행 문자로 분리
        std::vector<std::string> frameLines;
        std::istringstream stream(processedString);
        std::string line;
        while (std::getline(stream, line)) {
            // \r 제거
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            frameLines.push_back(line);
        }

        if (!frameLines.empty()) {
            _AnimationFrames.push_back(frameLines);
            frameCount++;
            PrintManager::GetInstance()->PrintLogLine(
                "Frame " + std::to_string(frameCount) + " loaded with " +
                std::to_string(frameLines.size()) + " lines",
                ELogImportance::DISPLAY);
        }

        pos++;
    }

    if (_AnimationFrames.empty()) {
        _LastError = "No frames loaded from JSON";
        _HasError = true;
        PrintManager::GetInstance()->PrintLogLine(_LastError, ELogImportance::WARNING);
        return false;
    }

    _CurrentFrame = 0;
    _IsAnimating = true;
    _IsDirty = true;
    _HasError = false;
    _LastError.clear();

    PrintManager::GetInstance()->PrintLogLine(
        "Successfully loaded " + std::to_string(_AnimationFrames.size()) + " frames from " + fileName,
        ELogImportance::DISPLAY);

    return true;
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
    // _IsDirty 체크 제거 - TextRenderer와 동일한 패턴

    // 애니메이션 모드
    if (_IsAnimating && !_AnimationFrames.empty()) {
        RenderFrame(buffer, bounds, _AnimationFrames[_CurrentFrame]);
    }
    // 단일 아트 모드
    else if (!_ArtLines.empty()) {
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

    // 아트 크기 검증 - 실제 화면 너비 계산 (UTF-8 코드포인트 기반)
    int artHeight = static_cast<int>(frame.size());
    int artWidth = 0;

    // 각 줄의 실제 화면 너비 계산
    for (const auto& line : frame) {
        int lineVisualWidth = 0;
        size_t i = 0;

        while (i < line.length()) {
            int charLen = 1;
            int codepoint = DecodeUTF8CodePoint(line, i, charLen);
            int visualWidth = GetCodePointVisualWidth(codepoint);

            lineVisualWidth += visualWidth;
            i += charLen;
        }

        artWidth = (std::max)(artWidth, lineVisualWidth);
    }

    // 패널보다 크면 경고
    if (artHeight > contentHeight || artWidth > contentWidth) {
        // 경고만 출력, 렌더링은 계속 진행
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

    // 시작 위치가 음수면 보정
    if (startX < contentX) startX = contentX;
    if (startY < contentY) startY = contentY;

    // 렌더링
    int currentY = startY;
    for (const auto& line : frame) {
        if (currentY >= contentY + contentHeight) break;
        if (currentY >= bounds.Y + bounds.Height - 1) break;

        buffer.WriteString(startX, currentY, line, _Color);
        currentY++;
    }
}
