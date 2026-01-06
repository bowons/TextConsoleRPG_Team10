#pragma once
#include <string>

// 동료 정보 구조체 (Companion.csv에서 로드)
struct CompanionData {
    std::string _CompanionId;      // 동료 ID (c_war_good, c_mag_mid 등)
    std::string _Name;             // 이름
    std::string _JobType;          // 직업 타입 (warrior, mage, archer, priest)
    int _HP;                       // 기본 HP
    int _MP;                       // 기본 MP
    int _Atk;                      // 기본 공격력
    int _Def;                      // 기본 방어력
    int _Dex;                      // 기본 민첩성
    int _Luk;                      // 기본 행운
    float _CritRate;               // 치명타 확률
    float _AppearRate;             // 등장 확률 (0.0 ~ 1.0)
    std::string _AsciiFile;        // 아스키 아트 파일 경로
};