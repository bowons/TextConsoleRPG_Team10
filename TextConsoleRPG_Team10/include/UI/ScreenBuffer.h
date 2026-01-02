#pragma once
#include <vector>
#include <string>
#include <Windows.h>

// UTF-8 문자 정보
struct CharCell {
    char Bytes[4] = { ' ', 0, 0, 0 };  // UTF-8 바이트 (최대 3바이트 + null)
    int ByteCount = 1;            // 실제 바이트 수
    int VisualWidth = 1;   // 화면에서 차지하는 칸 수 (한글=2, 영문=1)
    WORD Attribute = 7;      // 색상 속성 (기본: LIGHT_GRAY)

    CharCell() = default;
    CharCell(char ch) : Bytes{ ch, 0, 0, 0 }, ByteCount(1), VisualWidth(1) {}
};

// 화면 버퍼 (106x65)
class ScreenBuffer
{
private:
    static const int DEFAULT_WIDTH = 106;
    static const int DEFAULT_HEIGHT = 65;

    int _Width;
    int _Height;
    std::vector<std::vector<CharCell>> _Buffer;  // [Y][X]

public:
    ScreenBuffer(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);

    // 버퍼 초기화
    void Clear();

    // 단일 문자 쓰기 (UTF-8 지원)
    // return: 실제로 쓴 칸 수
    int WriteChar(int x, int y, const char* utf8Char, int byteCount, WORD attribute = 7);

    // 문자열 쓰기 (자동 개행 없음, 영역 밖은 무시)
    // return: 쓴 문자 수
    int WriteString(int x, int y, const std::string& text, WORD attribute = 7);

    // 영역 채우기
    void FillRect(int x, int y, int width, int height, char fillChar = ' ', WORD attribute = 7);

// 테두리 그리기 (싱글 라인 박스)
    void DrawBox(int x, int y, int width, int height, WORD attribute = 7);

    // 화면에 실제 렌더링 (더블 버퍼링)
    void Render();

    // Getter
    inline int GetWidth() const { return _Width; }
    inline int GetHeight() const { return _Height; }
    CharCell& GetCell(int x, int y);
    const CharCell& GetCell(int x, int y) const;

private:
    // UTF-8 문자 파싱
    static int ParseUTF8Char(const char* str, int& byteCount, int& visualWidth);
};
