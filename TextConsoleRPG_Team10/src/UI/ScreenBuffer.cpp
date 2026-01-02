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
    COORD coord = { 0, 0 };

    // 커서를 (0,0)으로 이동
    SetConsoleCursorPosition(hConsole, coord);

    // 커서 숨기기
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    // 한 번에 출력할 문자열 생성
    std::string output;
    output.reserve(_Width * _Height * 3);  // UTF-8 고려

    WORD currentAttr = 7;
    for (int y = 0; y < _Height; ++y) {
    for (int x = 0; x < _Width; ++x) {
  const CharCell& cell = _Buffer[y][x];

   // 색상 변경
 if (cell.Attribute != currentAttr) {
    // 실시간 색상 변경은 성능 저하 → 일단 무시 또는 ANSI 이스케이프 코드 사용
      currentAttr = cell.Attribute;
  }

       // 문자 출력
     if (cell.ByteCount > 0 && cell.Bytes[0] != '\0') {
      for (int i = 0; i < cell.ByteCount; ++i) {
           output += cell.Bytes[i];
    }
            }
            else if (cell.Bytes[0] == '\0') {
           // 이전 칸에 속하는 경우 (한글 2칸 처리) → 공백 출력하지 않음
    continue;
    }
            else {
       output += ' ';
            }
        }
  if (y < _Height - 1) {
    output += '\n';
        }
    }

    // 한 번에 출력
    std::cout << output << std::flush;
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
