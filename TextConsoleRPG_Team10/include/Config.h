#pragma once

// ===================== Path Configurations =====================
constexpr const char* DEFAULT_RESOURCE_PATH = "./Resources";  // 기본 리소스 경로

// ===================== Resource Folder Structure =====================
// 리소스 타입별 폴더 구조 (확장 가능)
constexpr const char* ANIMATIONS_FOLDER = "Animations/";      // 애니메이션 리소스
constexpr const char* CHARACTERS_FOLDER = "Characters/";      // 캐릭터 데이터
constexpr const char* MAPS_FOLDER = "Maps/";                  // 맵 데이터
constexpr const char* MONSTERS_FOLDER = "Monsters/";          // 몬스터 데이터
constexpr const char* UI_FOLDER = "UI/";                      // UI 리소스
constexpr const char* ITEMS_FOLDER = "Items/";                // 아이템 데이터
constexpr const char* Sound_FOLDER = "Sound/";               // 사운드 데이터

// ===================== File Names =====================
constexpr const char* ITEMS_CSV = "Items.csv";                // 아이템 목록 CSV
constexpr const char* MONSTERS_CSV = "Monsters.csv";          // 몬스터 목록 CSV
constexpr const char* MAPS_CSV = "Maps.csv";                  // 맵 목록 CSV

// ===================== UI Drawer Configurations =====================
constexpr int UI_SCREEN_WIDTH = 106;            // UI 화면 너비
constexpr int UI_SCREEN_HEIGHT = 65;          // UI 화면 높이
constexpr float UI_TARGET_FPS = 30.0f;              // UI 목표 FPS

// ===================== Other Configurations =====================
constexpr const char* ERR_INDEX = "ERR_INDEX";               // 인덱스 오류 문자열
constexpr const char* ERR_NULL = "ERR_NULL";                 // 널 포인터 오류 문자열