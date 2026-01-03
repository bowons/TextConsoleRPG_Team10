#include "../../include/UI/ScreenBuffer.h"
#include <iostream>
#include <algorithm>

ScreenBuffer::ScreenBuffer(int width, int height)
    : _Width(width), _Height(height)
{
    _Buffer.resize(_Height);
    for (auto& row : _Buffer) {
        row.resize(_Width);
    }
}

void ScreenBuffer::Clear()
{
    for (auto& row : _Buffer) {
        for (auto& cell : row) {
            cell = CharCell(' ');
        }
    }
}

int ScreenBuffer::ParseUTF8Char(const char* str, int& byteCount, int& visualWidth)
{
    if (!str || str[0] == '\0') {
        byteCount = 0;
        visualWidth = 0;
        return -1;
    }

    unsigned char ch = static_cast<unsigned char>(str[0]);

    // UTF-8 첫 바이트 분석
    if (ch < 0x80) {
        // ASCII (1바이트)
        byteCount = 1;
        visualWidth = 1;
    }
    else if ((ch & 0xE0) == 0xC0) {
        // 2바이트 문자
        byteCount = 2;
        visualWidth = 2;
    }
    else if ((ch & 0xF0) == 0xE0) {
        // 3바이트 문자 (한글 등)
        byteCount = 3;
        visualWidth = 2;
    }
    else if ((ch & 0xF8) == 0xF0) {
        // 4바이트 문자 (이모지 등)
        byteCount = 4;
        visualWidth = 2;
    }
    else {
        // 잘못된 UTF-8
        byteCount = 1;
        visualWidth = 1;
    }

    return 0;
}

int ScreenBuffer::WriteChar(int x, int y, const char* utf8Char, int byteCount, WORD attribute)
{
    if (x < 0 || y < 0 || y >= _Height) return 0;

    int visualWidth = 1;
    if (byteCount == 3 || byteCount == 2) {
        visualWidth = 2;  // 한글 등 멀티바이트
    }

    // 영역 밖이면 무시
    if (x + visualWidth > _Width) return 0;

    CharCell& cell = _Buffer[y][x];
    for (int i = 0; i < byteCount && i < 4; ++i) {
        cell.Bytes[i] = utf8Char[i];
    }
    cell.ByteCount = byteCount;
    cell.VisualWidth = visualWidth;
    cell.Attribute = attribute;

    // 한글인 경우 다음 칸은 빈칸으로 표시 (2칸 차지)
    if (visualWidth == 2 && x + 1 < _Width) {
        CharCell& nextCell = _Buffer[y][x + 1];
        nextCell.Bytes[0] = '\0';  // 마커: 이전 칸에 속함
        nextCell.ByteCount = 0;
        nextCell.VisualWidth = 0;
        nextCell.Attribute = attribute;
    }

    return visualWidth;
}

int ScreenBuffer::WriteString(int x, int y, const std::string& text, WORD attribute)
{
    if (y < 0 || y >= _Height) return 0;

    int currentX = x;
    int charCount = 0;
    size_t i = 0;

    while (i < text.length() && currentX < _Width) {
        int byteCount = 1;
        int visualWidth = 1;

        ParseUTF8Char(&text[i], byteCount, visualWidth);

        // 개행 처리
        if (text[i] == '\n') {
            break;  // 자동 개행 없음
        }

        // 쓸 공간 확인
        if (currentX + visualWidth > _Width) {
            break;
        }

        int written = WriteChar(currentX, y, &text[i], byteCount, attribute);
        currentX += written;
        i += byteCount;
        charCount++;
    }

    return charCount;
}

void ScreenBuffer::FillRect(int x, int y, int width, int height, char fillChar, WORD attribute)
{
    for (int row = y; row < y + height && row < _Height; ++row) {
        for (int col = x; col < x + width && col < _Width; ++col) {
            if (row >= 0 && col >= 0) {
                WriteChar(col, row, &fillChar, 1, attribute);
            }
        }
    }
}

void ScreenBuffer::DrawBox(int x, int y, int width, int height, WORD attribute)
{
    if (width < 2 || height < 2) return;

    // Box Drawing Characters (ASCII 호환)
    const char* topLeft = "+";
    const char* topRight = "+";
    const char* bottomLeft = "+";
    const char* bottomRight = "+";
    const char* horizontal = "-";
    const char* vertical = "|";

    // 상단
    WriteChar(x, y, topLeft, 1, attribute);
    for (int i = 1; i < width - 1; ++i) {
        WriteChar(x + i, y, horizontal, 1, attribute);
    }
    WriteChar(x + width - 1, y, topRight, 1, attribute);

    // 측면
    for (int i = 1; i < height - 1; ++i) {
        WriteChar(x, y + i, vertical, 1, attribute);
        WriteChar(x + width - 1, y + i, vertical, 1, attribute);
    }

    // 하단
    WriteChar(x, y + height - 1, bottomLeft, 1, attribute);
    for (int i = 1; i < width - 1; ++i) {
        WriteChar(x + i, y + height - 1, horizontal, 1, attribute);
    }
    WriteChar(x + width - 1, y + height - 1, bottomRight, 1, attribute);
}

void ScreenBuffer::Render()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // 커서 숨기기
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    bool wasCursorVisible = cursorInfo.bVisible;
    if (wasCursorVisible) {
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

    // WriteConsoleOutputW를 사용한 방식 (유니코드 지원)
    std::vector<CHAR_INFO> consoleBuffer(_Width * _Height);

    for (int y = 0; y < _Height; ++y) {
        for (int x = 0; x < _Width; ++x) {
            const CharCell& cell = _Buffer[y][x];
            CHAR_INFO& charInfo = consoleBuffer[y * _Width + x];

            // 문자 설정
            if (cell.ByteCount == 1 && cell.Bytes[0] != '\0') {
                // ASCII
                charInfo.Char.UnicodeChar = static_cast<wchar_t>(cell.Bytes[0]);
                charInfo.Attributes = cell.Attribute;
            }
            else if (cell.ByteCount > 1 && cell.Bytes[0] != '\0') {
                // UTF-8 -> UTF-16 변환 (한글 등)
                std::string utf8Str(cell.Bytes, cell.ByteCount);
                int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
                wchar_t wideChar = L' ';

                if (wideLen > 0) {
                    std::wstring wideStr(wideLen, 0);
                    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideLen);
                    wideChar = wideStr[0];
                }

                charInfo.Char.UnicodeChar = wideChar;
                charInfo.Attributes = cell.Attribute | COMMON_LVB_LEADING_BYTE;

                // 다음 칸 처리 (Trailing Byte)
                if (x + 1 < _Width) {
                    x++;  // 다음 칸으로 이동
                    CHAR_INFO& nextCharInfo = consoleBuffer[y * _Width + x];
                    nextCharInfo.Char.UnicodeChar = wideChar;
                    nextCharInfo.Attributes = cell.Attribute | COMMON_LVB_TRAILING_BYTE;
                }
            }
            else if (cell.Bytes[0] == '\0') {
                // 이미 처리된 한글의 두 번째 칸
            // 위에서 x++로 건너뛰므로 여기 도달하지 않음
                charInfo.Char.UnicodeChar = L' ';
                charInfo.Attributes = cell.Attribute;
            }
            else {
                charInfo.Char.UnicodeChar = L' ';
                charInfo.Attributes = cell.Attribute;
            }
        }
    }

    // WriteConsoleOutputW로 한 번에 출력 (깜빡임 없음 + 유니코드 지원)
    COORD bufferSize = { static_cast<SHORT>(_Width), static_cast<SHORT>(_Height) };
    COORD bufferCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, static_cast<SHORT>(_Width - 1), static_cast<SHORT>(_Height - 1) };

    SetConsoleCursorPosition(hConsole, { 0, 0 });
    WriteConsoleOutputW(hConsole, consoleBuffer.data(), bufferSize, bufferCoord, &writeRegion);

    // 커서 복원
    if (wasCursorVisible) {
        cursorInfo.bVisible = TRUE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }
}

CharCell& ScreenBuffer::GetCell(int x, int y)
{
    static CharCell dummy;
    if (x < 0 || x >= _Width || y < 0 || y >= _Height) {
        return dummy;
    }
    return _Buffer[y][x];
}

const CharCell& ScreenBuffer::GetCell(int x, int y) const
{
    static CharCell dummy;
    if (x < 0 || x >= _Width || y < 0 || y >= _Height) {
        return dummy;
    }
    return _Buffer[y][x];
}
