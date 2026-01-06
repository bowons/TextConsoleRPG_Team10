#pragma once
#include <string>

// ===== 간소화된 배틀 애니메이션 인터페이스 =====
class IBattleAnimationCallback
{
public:
    virtual ~IBattleAnimationCallback() = default;

    // ===== 패널에 애니메이션 설정 =====
    // panelName: 패널 이름 ("Animation", "Enemy", "CharArt0~3")
// animJsonFile: 애니메이션 JSON 파일명 (확장자 제외)
    // duration: 대기 시간 (초, 0이면 대기하지 않음)
    virtual void SetPanelAnimation(const std::string& panelName,
        const std::string& animJsonFile,
        float duration = 0.0f) = 0;

    // ===== 패널에 정적 아스키 아트 설정 =====
    // panelName: 패널 이름
    // artTxtFile: 아스키 아트 TXT 파일명 (확장자 제외)
    virtual void SetPanelArt(const std::string& panelName,
        const std::string& artTxtFile) = 0;

    // ===== UI 갱신 =====
    virtual void UpdatePartyDisplay() = 0;
    virtual void UpdateMonsterDisplay() = 0;
    virtual void RefreshBattleUI() = 0;
};

