#pragma once
#include "../UIScene.h"
#include <string>
#include <vector>

// 직업 정보 구조체
struct ClassData {
    std::string _ClassId;        // 직업 ID (warrior, mage, archer, priest)
    std::string _Role;           // 역할 (Tank, DPS, Support)
    std::string _RoleDesc;       // 특징 설명
    int _HP;                     // 초기 HP
    int _MP;                     // 초기 MP
    int _Atk;                    // 초기 공격력
    int _Def;                    // 초기 방어력
    int _Dex;                    // 초기 민첩성
    int _Luk;                    // 초기 행운
    float _CriticalRate;         // 치명타 확률
    std::string _AsciiFile;      // 아스키 아트 파일 경로
};

// 직업 스킬 정보 구조체
struct ClassSkillData {
    std::string _Skill1Name;     // 스킬1 이름
    std::string _Skill1Desc;     // 스킬1 설명
    std::string _Skill2Name;     // 스킬2 이름
    std::string _Skill2Desc;     // 스킬2 설명
    std::string _GrowthDesc;     // 성장 로직 설명
};

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
