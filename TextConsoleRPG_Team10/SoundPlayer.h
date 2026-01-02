#pragma once
#include "include/Singleton.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <windows.h>

// 데이터 중심의 구조체 유지
struct SoundInstance {
    std::string Alias;    // MCI 제어용 별칭
    bool bIsLoop;         // 반복 재생 여부
    float Volume;         // 현재 설정된 볼륨

    SoundInstance(std::string InAlias, bool InLoop, float InVol)
        : Alias(InAlias), bIsLoop(InLoop), Volume(InVol) {
    }
};

class SoundPlayer : public Singleton<SoundPlayer> {
private:
    SoundPlayer() : _IsInitialized(false) {}
    friend class Singleton<SoundPlayer>;

    bool _IsInitialized;
    // 활성화된 사운드를 별칭(Alias)으로 관리
    std::unordered_map<std::string, std::shared_ptr<SoundInstance>> _ActiveSounds;

public:
    // 초기화: DataManager 준비 상태 확인
    bool Initialize();

    // 단발성 효과음 재생 (자동 별칭 생성)
    void Play(const std::string& FileName, float Volume = 1.0f);

    // 반복 재생 (BGM 등, 이름을 직접 지정하여 Stop 시 사용)
    void PlayLoop(const std::string& FileName, const std::string& UniqueAlias, float Volume = 0.5f);

    // 제어 및 정리 함수
    void Stop(const std::string& Alias);
    void StopAll();
    void SetVolume(const std::string& Alias, float Volume);

    // 매 프레임 호출: 수명이 다한 사운드 리소스 해제
    void Update();

private:
    std::string GetFullSoundPath(const std::string& FileName);
};