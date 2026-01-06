#pragma once
#include "../UIScene.h"
#include <vector>

// 스토리 진행 Scene
class StoryProgressScene : public UIScene
{
private:
    int _Floor; // 현재 Floor 인덱스
    int _CurrentStoryIndex;  // 현재 스토리 단계
    bool _TextComplete;  // 텍스트 출력 완료 여부
    // 아예 출력할 데이터만 따로 저장하기로 변경
    std::vector<std::string> _StoryTexts;
    std::vector<std::string> _BossPhase2Texts;
    std::vector<int> _LineColor;
    std::string _BGMID;

public:
    StoryProgressScene();
    ~StoryProgressScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
private:
    void UpdateUIWithCurrentStory();
    void GetStoriesData(int FloorIndex);
    void SplitText(std::vector<std::string>& OutList, const std::string& Content, const std::string& Delimiter);
    std::vector<std::vector<std::string>> CheckPartyInfo();

    void SetArtImage(std::string FileName);
    void CheckArtUpdate();
};
