# 애니메이션 콜백 시스템 - 간소화 버전

## 개요
`BattleManager`에서 `BattleScene`의 UI를 제어하는 **단순한** 콜백 시스템

---

## 사용 가능한 함수 (2개만!)

### 1. **애니메이션 설정** (JSON)
```cpp
// panelName: "Animation", "Enemy", "CharArt0~3"
// animJsonFile: 애니메이션 파일명 (확장자 제외)
// duration: 대기 시간 (초, 0이면 대기 안 함)
_AnimationCallback->SetPanelAnimation("Enemy", "BossAttack", 1.0f);
```

### 2. **정적 아트 설정** (TXT)
```cpp
// panelName: "Animation", "Enemy", "CharArt0~3"
// artTxtFile: 아스키 아트 파일명 (확장자 제외)
_AnimationCallback->SetPanelArt("Enemy", "BossPhase2");
```

### 3. **UI 갱신** (보너스)
```cpp
_AnimationCallback->UpdatePartyDisplay();      // 파티 패널
_AnimationCallback->UpdateMonsterDisplay();    // 몬스터 패널
_AnimationCallback->RefreshBattleUI();         // 전체 UI
```

---

## 실전 예시

### 보스 페이즈 전환
```cpp
// BattleManager.cpp - ProcessAttack()
if (_AnimationCallback)
{
    // 1. 페이즈 전환 애니메이션
    _AnimationCallback->SetPanelAnimation("Animation", "PhaseTransition", 1.5f);
    
    // 2. 보스 이미지 변경
    _AnimationCallback->SetPanelArt("Enemy", "BossEnraged");
    
    // 3. UI 갱신
    _AnimationCallback->RefreshBattleUI();
}
```

### 스킬 사용
```cpp
if (_AnimationCallback)
{
    // 스킬 애니메이션 재생 + 1초 대기
    _AnimationCallback->SetPanelAnimation("Animation", "Fireball", 1.0f);
    
// 몬스터 HP 갱신
    _AnimationCallback->UpdateMonsterDisplay();
}
```

### 파티원 상태 변화
```cpp
if (_AnimationCallback)
{
    // 2번째 파티원 기절 이미지
    _AnimationCallback->SetPanelArt("CharArt1", "Stunned");
    
    // 파티 패널 갱신
    _AnimationCallback->UpdatePartyDisplay();
}
```

---

## 패널 이름

| 패널 이름 | 설명 |
|---------|------|
| `Animation` | 중앙 애니메이션 영역 |
| `Enemy` | 몬스터 이미지 |
| `CharArt0` | 1번째 파티원 |
| `CharArt1` | 2번째 파티원 |
| `CharArt2` | 3번째 파티원 |
| `CharArt3` | 4번째 파티원 |

---

## 파일 구조

```
Resources/
├── Animations/       (JSON 애니메이션)
│   ├── Fireball.json
│   ├── BossAttack.json
│   └── ...
│
├── Monsters/         (몬스터 TXT 아트)
│   ├── Boss.txt
│   ├── BossPhase2.txt
│ └── ...
│
└── Characters/       (캐릭터 TXT 아트)
    ├── Warrior.txt
    ├── Stunned.txt
    └── ...
```

---

## 주의사항

1. **nullptr 체크 필수**
```cpp
if (_AnimationCallback)  // 항상 체크!
{
    _AnimationCallback->SetPanelArt(...);
}
```

2. **Duration의 의미**
   - `duration > 0`: 애니메이션 재생 후 해당 시간만큼 대기 (블로킹)
   - `duration = 0`: 즉시 설정 후 다음 코드 실행 (논블로킹)

3. **파일 경로 자동 감지**
   - `SetPanelAnimation()`: `Resources/Animations/` 폴더
   - `SetPanelArt()`: 패널 이름에 따라 자동 감지
     - `Enemy` → `Resources/Monsters/`
     - `CharArt*` → `Resources/Characters/`
     - 기타 → `Resources/Animations/`

---

## 체크리스트

**애니메이션 추가:**
- [ ] JSON 파일 생성 (`Resources/Animations/`)
- [ ] `_AnimationCallback` nullptr 체크
- [ ] `SetPanelAnimation()` 호출
- [ ] `duration` 설정 (0 = 논블로킹, > 0 = 블로킹)

**아트 교체:**
- [ ] TXT 파일 생성 (`Resources/Monsters/` 또는 `Resources/Characters/`)
- [ ] `_AnimationCallback` nullptr 체크
- [ ] `SetPanelArt()` 호출
- [ ] `UpdateXXXDisplay()` 호출로 UI 갱신

---

**이제 2개 함수만 기억하세요!** 🎉
- `SetPanelAnimation()` → JSON 애니메이션
- `SetPanelArt()` → TXT 아트 교체
