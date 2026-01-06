#pragma once
#include "Player.h"

struct ClassData;

class Archer : public Player
{
protected:
    void InitializeSkills() override;
  
    // 직업별 숙련도 성장 (민첩/치명타 숙련도 활용)
    void ApplyProficiencyGrowth() override;

public:
    Archer(const std::string& name, bool enableInventory = false);
    Archer(const ClassData& data, const std::string& name, bool enableInventory = false);
    ~Archer() override = default;
    
    // 아처 전용 스킬 선택 AI
    int SelectBestSkill(ICharacter* target) const override;
    
    // 어그로 최대 대사 오버라이드
    std::string GetAggroMaxDialogue() const override;
};
