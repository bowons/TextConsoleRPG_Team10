#pragma once
#include <string>

// 직업 정보 구조체
struct ClassData {
    std::string _ClassId;          // 직업 ID (warrior, mage, archer, priest)
    std::string _Role;             // 역할 (Tank, DPS, Support)
    std::string _RoleDesc;         // 특징 설명
    int _HP;                       // 초기 HP
    int _MP;                       // 초기 MP
    int _Atk;                      // 초기 공격력
    int _Def;                      // 초기 방어력
    int _Dex;                      // 초기 민첩성
    int _Luk;                      // 초기 행운
    float _CriticalRate;           // 치명타 확률
    std::string _AsciiFile;        // 캐릭터 아스키 아트 파일 경로
    std::string _AsciiFileSelect;  // 캐릭터 선택 씬 아스키 아트 파일 경로
};

// 직업 스킬 정보 구조체
struct ClassSkillData {
    std::string _Skill1Name;     // 스킬1 이름
    std::string _Skill1Desc;     // 스킬1 설명
    std::string _Skill2Name;     // 스킬2 이름
    std::string _Skill2Desc;     // 스킬2 설명
    std::string _GrowthDesc;     // 성장 로직 설명
};