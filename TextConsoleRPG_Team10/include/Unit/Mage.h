#pragma once
#include "Player.h"

struct ClassData;

class Mage : public Player
{
private:
    // ===== 마력 폭주 버프 시스템 =====
    bool _IsMagicOverloadActive;
    int _MagicOverloadRoundsRemaining;

protected:
    void InitializeSkills() override;

    // 직업별 숙련도 성장 (마법 숙련도로 공격력 증가)
    void ApplyProficiencyGrowth() override;

public:
    Mage(const std::string& name, bool enableInventory = false);
    Mage(const ClassData& data, const std::string& name, bool enableInventory = false);
    ~Mage() override = default;

    // 메이지 전용 스킬 선택 AI
    int SelectBestSkill(ICharacter* target) const override;

    // 어그로 최대 대사 오버라이드
    std::string GetAggroMaxDialogue() const override;

    // ===== 마력 폭주 시스템 =====
    inline bool IsMagicOverloadActive() const { return _IsMagicOverloadActive; }
    void ActivateMagicOverload(int rounds);
    void DeactivateMagicOverload();

    // ProcessRoundEnd 오버라이드 (마력 폭주 처리 추가)
    void ProcessRoundEnd() override;

    // ResetBuffs 오버라이드 (마력 폭주 초기화 추가)
    void ResetBuffs() override;
};
