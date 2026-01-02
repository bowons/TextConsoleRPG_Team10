#pragma once
#include "../Singleton.h"
#include <memory>
#include <map>
#include <string>
#include <chrono>

class Panel;
class ScreenBuffer;

// UI Drawer 메인 매니저
// 여러 패널을 관리하고 화면에 렌더링
class UIDrawer : public Singleton<UIDrawer>
{
private:
    std::unique_ptr<ScreenBuffer> _ScreenBuffer;
    std::map<std::string, std::unique_ptr<Panel>> _Panels;
    
    bool _IsActive = false;  // UI Drawer 활성화 여부
    float _TargetFPS = 30.0f;
    std::chrono::steady_clock::time_point _LastFrameTime;

private:
    UIDrawer();
    friend class Singleton<UIDrawer>;

    UIDrawer(const UIDrawer&) = delete;
    UIDrawer& operator=(const UIDrawer&) = delete;

public:
    ~UIDrawer();

    // 초기화 및 종료
    bool Initialize(int width = 106, int height = 65);
    void Shutdown();

    // 패널 관리
    Panel* CreatePanel(const std::string& id, int x, int y, int width, int height);
    Panel* GetPanel(const std::string& id);
    void RemovePanel(const std::string& id);
 void RemoveAllPanels();

    // 렌더링
    void Update();           // 애니메이션 업데이트 + 렌더링
    void Render();      // 화면에 출력
    void RedrawAll();        // 모든 패널 강제 재렌더링

    // UI 모드 제어
    void Activate();  // UI Drawer 모드 활성화
    void Deactivate();       // UI Drawer 모드 비활성화
    inline bool IsActive() const { return _IsActive; }

  // FPS 설정
    void SetTargetFPS(float fps) { _TargetFPS = fps; }

    // 화면 클리어
    void ClearScreen();

private:
    float CalculateDeltaTime();
};
