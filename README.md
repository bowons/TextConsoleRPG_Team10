# 🗼 Erebos Tower

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![C++](https://img.shields.io/badge/C++-17-00599C.svg?logo=c%2B%2B)
![Platform](https://img.shields.io/badge/platform-Windows-0078D6.svg?logo=windows)
![License](https://img.shields.io/badge/license-MIT-green.svg)

**[🎥 Gameplay Video]([https://youtu.be/VIDEO_ID](https://youtu.be/k-tPc7hbnwE))**  
**[📄 Game Design Document](https://www.notion.so/teamsparta/2de2dc3ef5148072bfe8e8df9bed6356)**
**[💻 GitHub 저장소](https://github.com/SpartChapter2Team10/TextConsoleRPG_Team10)**  

**Text-based Turn-based Strategy RPG · Roguelike**

[![플레이 영상](assets/video_thumbnail.png)](https://youtu.be/k-tPc7hbnwE)

> “강자는 모든 것을 잃고, 약자는 모든 것을 얻는다.”

</div>

---

## 📖 Overview

**Erebos Tower**는  
텍스트 기반 **턴제 전략 RPG + 로그라이크** 구조의 Windows 콘솔 게임입니다.

플레이어는 모든 능력을 잃은 지망생으로 시작해  
몬스터를 처치하며 얻은 힘만으로 성장하고,  
뒤틀린 탑의 최상층에서 보스를 처치하는 것이 목표입니다.

- **Genre**: Turn-based Strategy RPG, Roguelike  
- **Platform**: Windows Console  
- **Playtime**: 2~3 Hours  
- **Engine**: C++ (Custom Console Framework)

---

## 🎮 Core Features

### 🎭 Class System

4개의 기본 직업 중 하나를 선택해 플레이합니다.

- **Warrior**: 방어 및 어그로 중심의 탱커
- **Mage**: 고화력 마법 딜러
- **Archer**: DEX 기반 지속 딜러
- **Priest**: 힐과 버프에 특화된 서포터

---

### 📈 Action-based Growth System (FF2-style)

전투 중 **실제로 수행한 행동에 따라 스탯이 성장**합니다.

같은 직업이라도 플레이 방식에 따라  
완전히 다른 빌드가 형성되며,  
전투 선택이 곧 캐릭터의 성장 경로가 됩니다.

| Class | Growth Trigger |
|------|----------------|
| Warrior | 피격 시 HP / DEF 증가 |
| Mage | 마법 사용 시 MP / ATK 증가 |
| Archer | 공격 적중 시 DEX / Crit 증가 |
| Priest | 힐·버프 사용 시 효과 증가 |

---

### 🤝 Companion System

탐험 도중 조우한 캐릭터를  
동료로 영입할 수 있습니다.

각 동료는 직업별 성능 등급을 가지며,  
확률 기반 등장과 개별 스탯 차별화를 통해  
플레이마다 다른 파티 구성이 만들어집니다.

- 총 **12명**의 동료 캐릭터

---

### ⚔️ Tactical Turn-based Combat

전투는 **DEX 기반 턴제**로 진행됩니다.

- 행동 순서가 전략에 직접적인 영향
- 상황에 따라 스킬 자동 선택
- 100 단위 **Aggro 시스템**
- 전투 전 캐릭터별 아이템 사용 예약

---

### 🗺 Roguelike Node Map

게임은 **총 10개 층**으로 구성된  
분기형 노드 맵 구조를 따라 진행됩니다.



| Node | Description |
|------|-------------|
| **[*] Start** | 시작 지점 |
| **[M] Normal** | 일반 전투 |
| **[E] Elite** | 정예 전투 |
| **[?] Companion** | 동료 영입 이벤트 |
| **[BOSS] Boss** | 보스 전투 |
| **[UP] Next Floor** | 다음 층 이동 |


각 선택은 다음 전투와 성장에  
직접적인 영향을 줍니다.

---

### 👾 Monsters

- **Normal Monster**: 18종
- **Elite Monster**: 9종
- **Boss Monster**: 1종 (Phase 기반 전투)

---

### 🧪 Item & Shop System

전투와 탐험을 보조하는  
아이템 및 상점 시스템을 제공합니다.

- 총 **7종**의 전략 아이템
- 층별 상점 제공 (F2 키로 즉시 접근)
- 강력한 아이템은 수량 제한

---

## 🛠 Tech Stack

### 🧩 Development Environment

| Category | Details |
|--------|---------|
| **Language** | C++17 |
| **IDE** | Visual Studio 2022 |
| **Platform** | Windows Console |
| **Build Tool** | MSBuild |
| **VCS** | Git / GitHub |


---

### 🧩Architecture & Design
> 모든 주요 시스템은 Manager 중심의 Singleton 구조로 관리되며,  
> 전투·아이템·유닛 로직은 인터페이스 기반으로 분리되어 있습니다.

- **Design Pattern**: Singleton (Game / Battle / Shop / Input / Print / Data)
- **SOLID Principles** 적용
- Interface 기반 확장 구조
- Player / Monster 완전 치환 가능

```cpp
void BattleManager::ProcessTurn(ICharacter* atk, ICharacter* def) {
    atk->Attack(def);
}
```
### 🧩STL & Memory Management

- `vector`를 활용한 컬렉션 관리 (인벤토리, 상점, 데이터 목록)
- `shared_ptr`, `unique_ptr` 기반 객체 수명 관리
- 동적 할당 객체의 자동 해제를 통한 메모리 안정성 확보
- 전투, 아이템, 입력 검증 등 핵심 시스템 전반에 STL 적극 활용

### 📁 Data-driven Design
게임 밸런스와 콘텐츠는 CSV 기반 데이터로 관리됩니다.

```cpp
Directory Structure
/Resources
├─ Data (CSV)
│ ├─ Class
│ ├─ Companion
│ ├─ Enemy
│ │ ├─ Normal
│ │ ├─ Elite
│ │ └─ Boss
│ ├─ Items
│ └─ FloorScaling
└─ Maps
  └─ Floor1 ~ Floor10
```

## 🧱 Core Systems
- 유닛 시스템 (Player / Monster)
- 아이템 / 인벤토리 시스템
- 턴제 전투 시스템
- 로그라이크 노드 맵 시스템
- CSV 기반 밸런스 시스템

---
## 💻Build & Run

### ⚙️ Requirements
- Windows 10 이상
- Visual Studio 2019 이상
- C++17
git clone https://github.com/YourTeam/TextConsoleRPG_Team10.git

### 🎮 Controls
| 키 입력 | 동작 |
|------|------|
| ← / → | 좌우 이동 |
| ↑ / ↓ | 분기 선택 |
| SPACE / ENTER | 노드 진입 |
| F2 | 상점 열기 |
| ESC | 포커스 복귀 |

---

### ⚔️ 전투

| 키 입력 | 동작 |
|------|------|
| NUMPAD | 아이템 선택 |
| ENTER | 아이템 사용 예약 / 취소 |
| SPACE | 턴 진행 |

---

### 📊 Implementation Status
- Core Features     : 100%
- Code Size : 약 5,000 LOC
- Test Cases : 68 Test Cases
  
---

### 🔧 Troubleshooting
- 메모리 누수        → unique_ptr 적용
- CSV UTF-8 문제     → BOM 처리 로직 추가
- 콘솔 깜빡임        → Double Buffering 적용
