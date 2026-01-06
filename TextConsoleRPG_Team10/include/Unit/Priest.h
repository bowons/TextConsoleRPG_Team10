#pragma once
#include "Player.h"

struct ClassData;

class Priest : public Player
{
protected:
    void InitializeSkills() override;

public:
    Priest(const std::string& name, bool enableInventory = false);
    Priest(const ClassData& data, const std::string& name, bool enableInventory = false);
    ~Priest() override = default;
    
    // 프리스트 전용 스킬 선택 AI
    int SelectBestSkill(ICharacter* target) const override;
    
    // 어그로 최대 대사 오버라이드
    std::string GetAggroMaxDialogue() const override;
    
    // 회복 숙련도 성장 오버라이드
    void ApplyProficiencyGrowth() override;
};
