#include "SoundPlayer.h"
#include "include/Manager/DataManager.h" // 실제 DataManager.h 경로로 수정
#include <mmsystem.h>
#include <iostream>

#pragma comment(lib, "winmm.lib")

bool SoundPlayer::Initialize() {
    // DataManager의 싱글톤 인스턴스 존재 여부 체크 (Null 참조 방지)
    if (DataManager::GetInstance() == nullptr) return false;

    _IsInitialized = true;
    return true;
}

// wav 파일만 재생 가능, (44.1kHz / 1411kbps) 규격이 아니면 출력이 불가
void SoundPlayer::Play(const std::string& FileName, float Volume) {
    // 같은 파일도 여러 번 겹쳐 재생할 수 있도록 고유 별칭 생성
    std::string Alias = "Snd_" + std::to_string(GetTickCount64()) + "_" + std::to_string(rand() % 1000);
    std::string FullPath = GetFullSoundPath(FileName);

    // 1. 장치 열기 (경로 공백 대응을 위해 따옴표 포함)
    std::string OpenCmd = "open \"" + FullPath + "\" type mpegvideo alias " + Alias;
    if (mciSendStringA(OpenCmd.c_str(), NULL, 0, NULL) == 0) {
        // 2. 볼륨 설정
        SetVolume(Alias, Volume);
        // 3. 처음부터 재생
        std::string PlayCmd = "play " + Alias + " from 0";
        mciSendStringA(PlayCmd.c_str(), NULL, 0, NULL);

        _ActiveSounds[Alias] = std::make_shared<SoundInstance>(Alias, false, Volume);
    }
}

// wav 파일만 재생 가능, (44.1kHz / 1411kbps) 규격이 아니면 출력이 불가
void SoundPlayer::PlayLoop(const std::string& FileName, const std::string& UniqueAlias, float Volume) {
    std::string FullPath = GetFullSoundPath(FileName);

    // 1. 기존에 같은 별칭으로 열린 장치가 있다면 '확실하게' 먼저 닫습니다.
    // (이 과정이 없으면 277 Internal Error가 발생합니다.)
    mciSendStringA(("close " + UniqueAlias).c_str(), NULL, 0, NULL);

    // 2. 장치 열기 (type waveaudio나 mpegvideo를 명시하지 않는 것이 가장 호환성이 좋습니다.)
    std::string OpenCmd = "open \"" + FullPath + "\" type mpegvideo alias " + UniqueAlias;

    MCIERROR err = mciSendStringA(OpenCmd.c_str(), NULL, 0, NULL);
    if (err != 0) {
        std::cout << "[MCI ERROR] PlayLoop Open Failed: " << err << std::endl;
        return;
    }

    // 3. 볼륨 설정
    SetVolume(UniqueAlias, Volume);

    // 4. 반복 재생 명령 실행 (repeat 키워드 추가)
    std::string PlayCmd = "play " + UniqueAlias + " repeat";
    err = mciSendStringA(PlayCmd.c_str(), NULL, 0, NULL);

    if (err == 0) {
        // 관리 목록에 추가
        _ActiveSounds[UniqueAlias] = std::make_shared<SoundInstance>(UniqueAlias, true, Volume);
        std::cout << "[DEBUG] BGM 재생 성공: " << UniqueAlias << std::endl;
    }
}

void SoundPlayer::Update() {
    // 재생이 끝난 사운드 인스턴스 정리 (메모리 및 장치 핸들 누수 방지)
    for (auto it = _ActiveSounds.begin(); it != _ActiveSounds.end(); ) {
        char buffer[128];
        std::string StatusCmd = "status " + it->first + " mode";
        mciSendStringA(StatusCmd.c_str(), buffer, sizeof(buffer), NULL);

        // 재생이 멈췄고 루프 사운드가 아니라면 장치를 닫음
        if (std::string(buffer) == "stopped" && !it->second->bIsLoop) {
            mciSendStringA(("close " + it->first).c_str(), NULL, 0, NULL);
            it = _ActiveSounds.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SoundPlayer::SetVolume(const std::string& Alias, float Volume) {
    int Vol = static_cast<int>(Volume * 1000.0f); // MCI 볼륨 범위: 0~1000
    std::string VolCmd = "setaudio " + Alias + " volume to " + std::to_string(Vol);
    mciSendStringA(VolCmd.c_str(), NULL, 0, NULL);
}

void SoundPlayer::Stop(const std::string& Alias) {
    mciSendStringA(("stop " + Alias).c_str(), NULL, 0, NULL);
    mciSendStringA(("close " + Alias).c_str(), NULL, 0, NULL);
    _ActiveSounds.erase(Alias);
}

void SoundPlayer::StopAll()
{
    for (auto it = _ActiveSounds.begin(); it != _ActiveSounds.end(); )
    {
        mciSendStringA(("stop " + it->first).c_str(), NULL, 0, NULL);
        mciSendStringA(("close " + it->first).c_str(), NULL, 0, NULL);
        it = _ActiveSounds.erase(it);
    }
}

std::string SoundPlayer::GetFullSoundPath(const std::string& FileName) {
    // DataManager의 리소스 경로 시스템 활용
    return DataManager::GetInstance()->GetResourcePath("Sound") + FileName;
}