# pragma once
#include "ICharacter.h"
#include "../Item/Inventory.h"
#include "../Skill/ISkill.h"
#include <string>
#include <memory>
#include <vector>
#include <map>

struct ClassData;  // 전방 선언

class Player : public ICharacter
{
private:
    int _CurrentExp;
    int _MaxExp;
    int _Gold;

    // 인벤토리 (메인 플레이어만 활성화)
    // nullptr인 경우 인벤토리 비활성화 (동료 캐릭터)
    std::unique_ptr<Inventory> _Inventory;

    // 각 버프별 라운드 관리
    int _AtkBuffRoundsRemaining;
    int _DefBuffRoundsRemaining;
    int _DexBuffRoundsRemaining;
    int _LukBuffRoundsRemaining;
    int _CriticalRateBuffRoundsRemaining;

    // ===== 스킬 시스템 =====
    std::vector<std::unique_ptr<ISkill>> _Skills;  // 보유 스킬 목록
    std::map<std::string, int> _SkillCooldowns;    // 스킬명 → 남은 쿨타임

    // ===== FF2 스타일 숙련도 시스템 =====
    int _HPProficiency;      // HP 숙련도 (피격 시 증가)
    int _MPProficiency;      // MP 숙련도 (마법 사용 시 증가)
    int _AtkProficiency;     // 공격 숙련도 (공격 시 증가)
    int _DefProficiency;     // 방어 숙련도 (피격 시 증가)
    int _DexProficiency;     // 민첩성 숙련도 (민첩 행동 시 증가)
    int _MagicProficiency;   // 마법 숙련도 (마법 사용 시 증가)

protected:
    // ===== 어그로 시스템 (자식 클래스에서 초기화 필요) =====
    int _AggroValue;  // 어그로 수치 (0~100)
    
    // 어그로 고정 시스템 (포효 스킬용)
    bool _IsAggroLocked = false;  // 어그로 고정 여부
    int _AggroLockRoundsRemaining = 0;  // 어그로 고정 남은 턴
    int _LockedAggroValue = 0;  // 고정된 어그로 값
    
    // 숙련도 포인트 (전투 종료 시 정산) - 자식 클래스에서 접근 가능
    int _DamageTakenThisBattle;
    int _MPSpentThisBattle;
    int _CriticalHitsThisBattle;
    int _HealingDoneThisBattle;
    
    // 직업별 초기화 (각 직업 클래스에서 구현)
    virtual void InitializeSkills() = 0;
    
    // CSV 데이터로 스탯 초기화 (각 직업 생성자에서 호출)
    void ApplyClassData(const ClassData& data);

public:
    // 기본 생성자 (기존 방식)
    // enableInventory: true = 인벤토리 활성화 (메인 플레이어)
    //              false = 인벤토리 비활성화 (동료 캐릭터)
    Player(const std::string& Name, bool enableInventory = false);
    
    // CSV 기반 생성자
    // data: Class.csv에서 로드한 직업 데이터
    // playerName: 플레이어 이름
    // enableInventory: 인벤토리 활성화 여부
    Player(const ClassData& data, const std::string& playerName, bool enableInventory);

    // 가상 소멸자
    virtual ~Player() {}

    int TakeDamage(ICharacter* Attacker, const int Amount) override;
    std::tuple<std::string, int> Attack(ICharacter* Target) const override;
    bool IsDead() const override;

    void CheckLevelUp();
    void ProcessLevelUp();
    void GainExp(const int Amount);
    void GainGold(const int Amount);

    // 아이템 사용 (안전)
    // 반환: 성공 시 true, 실패 시 false (인벤토리 없음 포함)
    bool UseItem(const int SlotIndex);

    inline int GetGold() const { return _Gold; }
    inline int GetMaxGold() const { return _MaxExp; }
    inline int GetExp() const { return _CurrentExp; }
    inline int GetMaxExp() const { return _MaxExp; }

    // 인벤토리 접근 (안전)
    // outInventory: 인벤토리 포인터 저장 (출력 매개변수)
    // 반환: 인벤토리가 있으면 true + outInventory에 포인터 저장
    // 없으면 false + outInventory는 nullptr
    bool TryGetInventory(Inventory*& outInventory);

    // 범용 스탯 수정 메서드 - 아이템이나 버프/디버프 등에서 사용
    void ModifyHP(int Amount);   // HP 증감 (최대치 제한)
    void ModifyMaxHP(int Amount);   // 최대 HP 증감
    void ModifyMP(int Amount);       // MP 증감 (최대치 제한)
    void ModifyMaxMP(int Amount);   // 최대 MP 증감
    void ModifyAtk(int Amount);  // 공격력 증감
    void ModifyDef(int Amount);     // 방어력 증감
    void ModifyDex(int Amount);  // 민첩성 증감
    void ModifyLuk(int Amount);     // 운 증감
    void ModifyCriticalRate(float Amount);  // 치명타율 증감
    void ModifyGold(int Amount);// 골드 증감

    // 버프 관리 메서드
    void ApplyTempAtkBuff(int Amount, int Rounds);
    void ApplyTempDefBuff(int Amount, int Rounds);
    void ApplyTempDexBuff(int Amount, int Rounds);
    void ApplyTempLukBuff(int Amount, int Rounds);
    void ApplyTempCriticalRateBuff(float Amount, int Rounds);
    virtual void ProcessRoundEnd();
    virtual void ResetBuffs();

    // 버프 포함 총 스탯 조회 메서드
    int GetTotalAtk() const;
    int GetTotalDef() const;
    int GetTotalDex() const;
    int GetTotalLuk() const;
    float GetTotalCriticalRate() const;

    // 플레이어 전용 공격 연출
    std::string GetAttackNarration() const override;

    // ===== 스킬 시스템 메서드 =====

    // 스킬 추가 (직업별 초기화 시 사용)
    void AddSkill(std::unique_ptr<ISkill> skill);

    // 스킬 사용 (MP 소모, 쿨타임 적용)
    // skillIndex: 스킬 배열 인덱스
    // target: 대상 (nullptr 가능 - 자가 버프용)
    // return: 스킬 결과 (성공/실패, 데미지 등)
    SkillResult UseSkill(int skillIndex, ICharacter* target);

    // 스킬 사용 가능 여부 체크 (MP, 쿨타임, 조건 확인)
    bool CanUseSkill(int skillIndex) const;

    // 최적의 스킬 선택 (AI용 - 조건 만족하는 스킬 중 우선순위 높은 것)
    // target: 대상
    // return: 사용할 스킬 인덱스 (-1이면 스킬 없음, 일반 공격 사용)
    virtual int SelectBestSkill(ICharacter* target) const;

    // 스킬 목록 조회
    inline const std::vector<std::unique_ptr<ISkill>>& GetSkills() const { return _Skills; }

    // 쿨타임 감소 (턴 종료 시 호출)
    void ProcessSkillCooldowns();

    // 특정 스킬의 남은 쿨타임 조회
    int GetSkillCooldown(const std::string& skillName) const;

    // ===== 어그로 시스템 =====
    inline int GetAggro() const { return _AggroValue; }
    void ModifyAggro(int amount);  // 어그로 증감 (0~100 제한, 고정 중이면 무시)
    void ResetAggro();  // 전투 시작 시 초기화 (전사 30, 나머지 0)
    virtual std::string GetAggroMaxDialogue() const;  // 어그로 100 도달 시 대사 (각 클래스에서 오버라이드)
    
    // 어그로 고정 시스템 (포효 스킬용)
    void LockAggro(int value, int rounds);  // 어그로를 특정 값으로 고정 (rounds 턴간)
    void UnlockAggro();  // 어그로 고정 해제
    inline bool IsAggroLocked() const { return _IsAggroLocked; }  // 어그로 고정 여부 확인

    // ===== FF2 숙련도 시스템 (나중에 구현) =====

    // 숙련도 증가 (행동 기반)
    void GainHPProficiency(int amount);   // 피격 시 호출
    void GainMPProficiency(int amount);   // 마법 사용 시 호출
    void GainAtkProficiency(int amount);  // 물리 공격 시 호출
    void GainDefProficiency(int amount);  // 피격 시 호출
    void GainDexProficiency(int amount);  // 민첩성 행동 시 호출
    void GainMagicProficiency(int amount);// 마법 사용 시 호출

    // 전투 종료 시 숙련도 정산
    void ProcessBattleEndProficiency();

    // 숙련도 추적 메서드
    void TrackDamageTaken(int amount);  // 피해 추적
    void TrackMPSpent(int amount);  // MP 소모 추적
    void TrackCriticalHit();    // 치명타 추적
    void TrackHealing(int amount); // 회복 추적

    // 숙련도 레벨업 체크 (일정 수치 도달 시 영구 스탯 증가)
    void CheckProficiencyGrowth();

    // 숙련도 조회
    inline int GetHPProficiency() const { return _HPProficiency; }
    inline int GetMPProficiency() const { return _MPProficiency; }
    inline int GetAtkProficiency() const { return _AtkProficiency; }
    inline int GetDefProficiency() const { return _DefProficiency; }
    inline int GetDexProficiency() const { return _DexProficiency; }
    inline int GetMagicProficiency() const { return _MagicProficiency; }

    // 직업별 특화 숙련도 성장 (각 클래스에서 오버라이드)
    virtual void ApplyProficiencyGrowth() {}

    void PlaySkillSound();
};
