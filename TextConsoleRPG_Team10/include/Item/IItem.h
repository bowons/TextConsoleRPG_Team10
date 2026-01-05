#pragma once
#include <string>
#include <memory>
#include "../Config.h"

class Player;

//아이템 인터페이스 클래스
class IItem
{
protected:
    std::string _ItemName; //아이템 이름
    int _Price; //아이템 가격
    int _EffectAmount; //아이템 효과 수치
    int _MaxCount; //아이템 최대 보유 가능한 개수

    // 예약 시스템 - 예약된 라운드 추적
    // -1: 예약 안 됨
    // 0 이상: 예약된 라운드 번호
    int _ReservedAtRound = -1;

public:
    //생성자
    IItem(std::string ItemName, int Price, int EffectAmount, int MaxCount)
        : _ItemName(ItemName), _Price(Price), _EffectAmount(EffectAmount), _MaxCount(MaxCount) {
    }
    virtual ~IItem() = default; //소멸자

    virtual void ApplyEffect(Player& Player) = 0; //아이템 사용 시 효과 적용

    // Clone 메서드: 동일한 타입의 새로운 아이템 인스턴스 생성
    // 인벤토리에서 여러 슬롯에 분산 저장 시 각 슬롯이 독립적인 아이템 객체를 소유하도록 함
    virtual std::unique_ptr<IItem> Clone() const = 0;

    // ===== 예약 시스템 =====

    // 아이템 예약 등록
    // reservedRound: 예약된 라운드 번호
    void Reserve(int reservedRound) { _ReservedAtRound = reservedRound; }

    // 아이템 예약 취소
    void CancelReservation() { _ReservedAtRound = -1; }

    // 예약 여부 확인
    // return: 예약되어 있으면 true
    bool IsReserved() const { return _ReservedAtRound >= 0; }

    // 예약된 라운드 반환
    // return: 예약된 라운드 (-1이면 예약 안 됨)
    int GetReservedRound() const { return _ReservedAtRound; }

    // 아이템 사용 가능 여부 판단 (조건 체크)
    // player: 사용자 
    // currentRound: 현재 라운드 (0부터 시작)
    // return: 사용 가능하면 true
    virtual bool CanUse(const Player& player, int currentRound) const = 0;

    // 사용 조건 설명 반환 (UI 표시용)
    // return: 조건 설명 문자열
    virtual std::string GetUseConditionDescription() const = 0;

    int GetMaxCount() const { return _MaxCount; } //아이템 최대 스택 수 반환
    std::string GetName() const { return _ItemName; } //아이템 이름 반환
    int GetPrice() const { return _Price; } //아이템 가격 반환
    int GetEffectAmount() const { return _EffectAmount; } //아이템 효과 수치 반환

    // ===== Setter 함수들 (CSV 데이터 적용용) =====
    void SetPrice(int price) { _Price = price; }
    void SetName(const std::string& name) { _ItemName = name; }
    void SetEffectAmount(int amount) { _EffectAmount = amount; }
    void SetMaxCount(int count) { _MaxCount = count; }
};