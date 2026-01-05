#include "../../include/UI/ScreenBuffer.h"
#include <iostream>
#include <algorithm>

// UTF-8 바이트 시퀀스를 유니코드 코드포인트로 변환
static int DecodeUTF8CodePoint(const char* str, int byteCount)
{
	if (!str || byteCount <= 0) return 0;

    unsigned char ch = static_cast<unsigned char>(str[0]);

    if (byteCount == 1) {
        return ch;  // ASCII
    }
    else if (byteCount == 2) {
     return ((ch & 0x1F) << 6) | (str[1] & 0x3F);
    }
    else if (byteCount == 3) {
   return ((ch & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
    }
    else if (byteCount == 4) {
        return ((ch & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
    }

    return 0;
}

// 유니코드 코드포인트의 시각적 너비 계산
static int GetCodePointVisualWidth(int codepoint)
{
    // ASCII 및 기본 라틴 문자: 1칸
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

    // 한글 자모 (U+1100 ~ U+11FF, U+3130 ~ U+318F): 2칸
    if ((codepoint >= 0x1100 && codepoint <= 0x11FF) ||
        (codepoint >= 0x3130 && codepoint <= 0x318F))
   return 2;

    // 한자 및 동아시아 문자 (CJK): 2칸
    if ((codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||  // CJK Unified Ideographs
        (codepoint >= 0x3400 && codepoint <= 0x4DBF) ||  // CJK Extension A
        (codepoint >= 0xF900 && codepoint <= 0xFAFF))    // CJK Compatibility Ideographs
        return 2;

  // 기타: 기본적으로 1칸
    return 1;
}

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

    // UTF-8 바이트 수만 먼저 결정
    if (ch < 0x80) {
        // ASCII (1바이트)
        byteCount = 1;
    }
    else if ((ch & 0xE0) == 0xC0) {
        // 2바이트 문자
        byteCount = 2;
    }
    else if ((ch & 0xF0) == 0xE0) {
        // 3바이트 문자 (한글, 점자 등)
        byteCount = 3;
    }
    else if ((ch & 0xF8) == 0xF0) {
        // 4바이트 문자 (이모지 등)
        byteCount = 4;
    }
    else {
        // 잘못된 UTF-8
  byteCount = 1;
        visualWidth = 1;
        return -1;
    }

    // 코드포인트로 변환 후 시각적 너비 계산
  int codepoint = DecodeUTF8CodePoint(str, byteCount);
    visualWidth = GetCodePointVisualWidth(codepoint);

    return 0;
}

int ScreenBuffer::WriteChar(int x, int y, const char* utf8Char, int byteCount, WORD attribute)
{
    if (x < 0 || y < 0 || y >= _Height) return 0;

    // 시각적 너비 계산 (코드포인트 기반)
    int codepoint = DecodeUTF8CodePoint(utf8Char, byteCount);
    int visualWidth = GetCodePointVisualWidth(codepoint);

    // 영역 밖이면 무시
    if (x + visualWidth > _Width) return 0;

    CharCell& cell = _Buffer[y][x];
    for (int i = 0; i < byteCount && i < 4; ++i) {
        cell.Bytes[i] = utf8Char[i];
 }
    cell.ByteCount = byteCount;
    cell.VisualWidth = visualWidth;
    cell.Attribute = attribute;

    // 2칸을 차지하는 문자인 경우 다음 칸은 빈칸으로 표시
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
     // UTF-8 -> UTF-16 변환 (한글, 점자 등)
        std::string utf8Str(cell.Bytes, cell.ByteCount);
       int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
       wchar_t wideChar = L' ';

    if (wideLen > 0) {
          std::wstring wideStr(wideLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wideStr[0], wideLen);
   wideChar = wideStr[0];
     }

       charInfo.Char.UnicodeChar = wideChar;

       // VisualWidth를 확인하여 2칸 문자만 특수 처리
   if (cell.VisualWidth == 2) {
        charInfo.Attributes = cell.Attribute | COMMON_LVB_LEADING_BYTE;

       // 다음 칸 처리 (Trailing Byte)
     if (x + 1 < _Width) {
       x++;  // 다음 칸으로 이동
   CHAR_INFO& nextCharInfo = consoleBuffer[y * _Width + x];
       nextCharInfo.Char.UnicodeChar = wideChar;
   nextCharInfo.Attributes = cell.Attribute | COMMON_LVB_TRAILING_BYTE;
     }
           }
        else {
// 1칸 문자 (점자 등)는 일반 속성만
         charInfo.Attributes = cell.Attribute;
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
