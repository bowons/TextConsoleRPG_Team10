#pragma once
#include "../Singleton.h"
#include "../Config.h"
#include <string>
#include <vector>

class Player;
struct ItemData;
struct MonsterSpawnData;

class DataManager : public Singleton<DataManager>
{
private:
    bool _IsInitialized;

private:
    DataManager() = default;
    friend class Singleton<DataManager>;

    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    // ===== 헬퍼 함수  =====
    inline std::string JoinPath(const std::string& path1, const std::string& path2) const
    {
        if (path1.empty()) return path2;
        if (path2.empty()) return path1;

        std::string out = path1;
        if (out.back() != '/' && out.back() != '\\') out.push_back('/');
        if (path2.front() == '/' || path2.front() == '\\') out += path2.substr(1);
        else out += path2;

        return out;
    }

    // ===== 리소스 폴더 경로 Getter =====
    inline std::string GetAnimationsPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), ANIMATIONS_FOLDER); }
    inline std::string GetCharactersPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), CHARACTERS_FOLDER); }
    inline std::string GetMapsPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), MAPS_FOLDER); }
    inline std::string GetMonstersPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), MONSTERS_FOLDER); }
    inline std::string GetUIPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), UI_FOLDER); }
    inline std::string GetItemsPath() const { return JoinPath(std::string(DEFAULT_RESOURCE_PATH), ITEMS_FOLDER); }

public:
    bool Initialize();

    // ===== 범용 파일 I/O (폴더 경로를 명시적으로 받음) =====
    std::string LoadTextFile(const std::string& folderPath, const std::string& fileName);
    std::vector<std::vector<std::string>> LoadCSVFile(const std::string& folderPath, const std::string& fileName);
    bool SaveTextFile(const std::string& folderPath, const std::string& fileName, const std::string& data);
    bool DeleteFile(const std::string& folderPath, const std::string& fileName);
    bool FileExists(const std::string& folderPath, const std::string& fileName);
    bool DirectoryExists(const std::string& dirPath);

    // ===== 특화된 리소스 로딩 함수들을 아래에 구현 =====
    std::vector<ItemData> LoadItemData(const std::string& fileName = "Items.csv");
    std::vector<MonsterSpawnData> LoadMonsterSpawnData(const std::string& fileName);
    std::tuple<std::string, std::string> GetRandomStageAndMonster();

    // === 편의 함수 =====
    std::string GetResourcePath(const std::string& resourceType) const;

    inline const bool IsInitialized() { return _IsInitialized; };
};
