#pragma once
#include "../UIScene.h"
#include <string>
#include <vector>
#include "../../Data/ClassData.h"

// 직업 선택 Scene
class CharacterSelectScene : public UIScene
{
private:
    int _CurrentSelection;                  // 현재 선택 중인 직업 인덱스
    std::vector<ClassData> _ClassDataList;  // 직업 데이터 목록
    std::vector<ClassSkillData> _SkillDataList; // 직업 스킬 데이터 목록

private:
    // CSV에서 직업 데이터 로드
    void LoadClassData();
    
    // 직업별 스킬 정보 초기화
    void InitializeSkillData();
    
    // UI 업데이트 함수들
    void UpdateClassImage(int selection);
    void UpdateClassInfo(int selection);
    void UpdateGuideMessage(int selection);
    void UpdateTitlePage(int selection);

public:
    CharacterSelectScene();
    ~CharacterSelectScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
};
