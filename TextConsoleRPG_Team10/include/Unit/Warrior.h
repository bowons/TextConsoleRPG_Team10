#pragma once
#include "Player.h"

struct ClassData;  // 전방 선언

class Warrior : public Player
{
protected:
    void InitializeSkills() override;
    
    // 직업별 숙련도 성장 (HP 숙련도 추가 보너스)
    void ApplyProficiencyGrowth() override;

public:
    // 기본 생성자 (하드코딩)
    Warrior(const std::string& name, bool enableInventory = false);
    
    // CSV 기반 생성자
    Warrior(const ClassData& data, const std::string& name, bool enableInventory = false);
    
    ~Warrior() override = default;

    // 워리어 전용 스킬 선택 AI
    int SelectBestSkill(ICharacter* target) const override;

    // 어그로 최대 대사 오버라이드
    std::string GetAggroMaxDialogue() const override;
    
    // ⭐ 어그로 초기화 오버라이드 (전사는 30으로 시작)
    void ResetAggro() override;
};
