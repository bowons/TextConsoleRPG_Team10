#include "../../include/UI/UIDrawer.h"
#include "../../include/UI/ScreenBuffer.h"
#include "../../include/UI/Panel.h"
#include "../../include/UI/IContentRenderer.h"
#include "../../include/Manager/PrintManager.h"
#include <Windows.h>
#include <thread>

UIDrawer::UIDrawer()
{
    _LastFrameTime = std::chrono::steady_clock::now();
}

UIDrawer::~UIDrawer()
{
    Shutdown();
}

bool UIDrawer::Initialize(int width, int height)
{
    try {
        _ScreenBuffer = std::make_unique<ScreenBuffer>(width, height);

        // 콘솔 설정
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // 콘솔 크기 설정
        COORD bufferSize = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
        SetConsoleScreenBufferSize(hConsole, bufferSize);

        SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

        // 커서 숨기기
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        PrintManager::GetInstance()->PrintLogLine(
            "UIDrawer initialized: " + std::to_string(width) + "x" + std::to_string(height),
            ELogImportance::DISPLAY);

        return true;
    }
    catch (const std::exception& ex) {
        PrintManager::GetInstance()->PrintLogLine(
            "UIDrawer initialization failed: " + std::string(ex.what()),
            ELogImportance::WARNING);
        return false;
    }
}

void UIDrawer::Shutdown()
{
    _Panels.clear();
    _ScreenBuffer.reset();
    _IsActive = false;

    // 커서 복원
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

Panel* UIDrawer::CreatePanel(const std::string& id, int x, int y, int width, int height)
{
    PanelBounds bounds(x, y, width, height);
    auto panel = std::make_unique<Panel>(id, bounds);
    Panel* ptr = panel.get();

    _Panels[id] = std::move(panel);
    return ptr;
}

Panel* UIDrawer::GetPanel(const std::string& id)
{
    auto it = _Panels.find(id);
    if (it != _Panels.end()) {
        return it->second.get();
    }
    return nullptr;
}

void UIDrawer::RemovePanel(const std::string& id)
{
    _Panels.erase(id);
}

void UIDrawer::RemoveAllPanels()
{
    _Panels.clear();
}

void UIDrawer::Update()
{
    if (!_IsActive || !_ScreenBuffer) return;

    float deltaTime = CalculateDeltaTime();

    bool needsRedraw = false;

    // 모든 패널의 콘텐츠 업데이트 (애니메이션 등)
    for (auto& pair : _Panels) {
        Panel* panel = pair.second.get();
        if (panel->GetContentRenderer()) {
            panel->GetContentRenderer()->Update(deltaTime);

            // 콘텐츠가 dirty면 패널도 dirty
            if (panel->GetContentRenderer()->IsDirty()) {
                panel->SetDirty();
                needsRedraw = true;
            }
        }
    }

    // 변경 사항이 있을 때만 렌더링
    if (needsRedraw) {
        Render();
    }

    // FPS 제한
    float targetFrameTime = 1.0f / _TargetFPS;
    float sleepTime = targetFrameTime - deltaTime;
    if (sleepTime > 0) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
    }
}

void UIDrawer::Render()
{
    if (!_ScreenBuffer) return;

    // 화면 클리어
    _ScreenBuffer->Clear();

    // 모든 패널 렌더링
    for (auto& pair : _Panels) {
        pair.second->RenderToBuffer(*_ScreenBuffer);
    }

    // 화면에 출력
    _ScreenBuffer->Render();
}

void UIDrawer::RedrawAll()
{
    for (auto& pair : _Panels) {
        pair.second->Redraw();
    }
    Render();
}

void UIDrawer::Activate()
{
    _IsActive = true;
    ClearScreen();
    PrintManager::GetInstance()->PrintLogLine("UI Drawer activated", ELogImportance::DISPLAY);
}

void UIDrawer::Deactivate()
{
    _IsActive = false;
    ClearScreen();
    PrintManager::GetInstance()->PrintLogLine("UI Drawer deactivated", ELogImportance::DISPLAY);
}

void UIDrawer::ClearScreen()
{
    // 콘솔 화면 완전 클리어
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    GetConsoleScreenBufferInfo(hConsole, &csbi);
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hConsole, ' ', dwConSize, coordScreen, &cCharsWritten);
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    FillConsoleOutputAttribute(hConsole, csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);
    SetConsoleCursorPosition(hConsole, coordScreen);
}

float UIDrawer::CalculateDeltaTime()
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - _LastFrameTime;
    _LastFrameTime = now;
    return elapsed.count();
}
