#pragma once
#include <string>
#include <tuple>

class ICharacter;
class Player;

// 스킬 타입
enum class ESkillType
{
    Physical,   // 물리 공격
    Magic,      // 마법 공격
    Buff,       // 버프
    Heal        // 힐
};

// 스킬 대상
enum class ESkillTarget
{
    Self,    // 자신
    SingleEnemy,    // 단일 적
    AllEnemies,     // 모든 적
    SingleAlly,     // 단일 아군
    AllAllies       // 모든 아군
};

// 스킬 결과 구조체
struct SkillResult
{
    std::string SkillName;      // 스킬명
    int Value;     // 데미지 또는 회복량
    int HitCount;     // 타격 횟수 (다단 히트용)
    bool Success;     // 성공 여부
    std::string Message;      // 추가 메시지 (크리티컬, 버프 적용 등)
};

class ISkill
{
protected:
    std::string _Name;           // 스킬명
    int _MPCost;         // 소모 MP
    int _Cooldown;             // 쿨타임 (턴)
    ESkillType _Type;            // 스킬 타입
    ESkillTarget _TargetType;    // 대상 타입
    int _BonusEffectAmount;      // 보너스 효과량 (숙련도에 따른 추가 데미지/회복량)

public:
    ISkill(const std::string& name, int mpCost, int cooldown,
        ESkillType type, ESkillTarget target)
        : _Name(name), _MPCost(mpCost), _Cooldown(cooldown),
        _Type(type), _TargetType(target), _BonusEffectAmount(0) {
    }

    virtual ~ISkill() = default;

    // 스킬 효과 계산 (데미지/회복량만 계산, 실제 적용은 BattleManager)
    // user: 스킬 사용자
    // target: 대상 (nullptr 가능 - 자가 버프)
    // return: 스킬 결과 (데미지, 타격 횟수 등)
    virtual SkillResult CalculateEffect(Player* user, ICharacter* target) = 0;

    // 스킬 사용 조건 체크 (각 직업별 스킬이 구현)
      // user: 스키 사용자
      // return: 조건 만족 시 true
    virtual bool CanActivate(const Player* user) const = 0;

    // 조건 설명 반환 (UI용)
    virtual std::string GetConditionDescription() const = 0;

    // Getter
    inline const std::string& GetName() const { return _Name; }
    inline int GetMPCost() const { return _MPCost; }
    inline int GetCooldown() const { return _Cooldown; }
    inline ESkillType GetType() const { return _Type; }
    inline ESkillTarget GetTargetType() const { return _TargetType; }
    
    // 보너스 효과량 관리
    inline int GetBonusEffectAmount() const { return _BonusEffectAmount; }
    inline void AddBonusEffectAmount(int amount) { _BonusEffectAmount += amount; }
    inline void SetBonusEffectAmount(int amount) { _BonusEffectAmount = amount; }
};
