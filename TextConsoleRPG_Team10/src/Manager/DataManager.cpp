#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Item/ItemData.h"
#include "../../include/Item/MonsterSpawnData.h"
#include "../../include/Data/StageData.h"
#include "../../include/Data/ClassData.h"  // 추가
#include "../../include/Data/FloorScalingData.h"  // 추가
#include "../../include/Manager/GameManager.h"
#include "../../include/Data/CompanionData.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <optional>
#include <random>


// Windows API 매크로 충돌 방지
#ifdef DeleteFile
#undef DeleteFile
#endif

// 초기화 확인 및 로그 출력용 헬퍼 함수
static void SafeLog(const std::string& message, ELogImportance importance = ELogImportance::WARNING)
{
    PrintManager* pm = PrintManager::GetInstance();
    if (pm)
    {
        pm->PrintLogLine(message, importance);
    }
    else
    {
        std::cerr << "[DataManager] " << message << std::endl;
    }
}

// ===== Path가 추가 될 때 마다 GetResourcePath 함수도 수정 필요 =====
std::string DataManager::GetResourcePath(const std::string& resourceType) const
{
    if (resourceType == "Animations") return GetAnimationsPath();
    if (resourceType == "Characters") return GetCharactersPath();
    if (resourceType == "Maps") return GetMapsPath();
    if (resourceType == "Monsters") return GetMonstersPath();
    if (resourceType == "UI") return GetUIPath();
    if (resourceType == "Items") return GetItemsPath();
    if (resourceType == "Sound") return GetSoundPath();
    if (resourceType == "Stages") return GetStagesPath();

    SafeLog("DataManager::GetResourcePath - Unknown resource type: " + resourceType);
    return "";
}

// ===== Path가 추가될 때 마다 RequiredDirs도 수정 필요 =====
bool DataManager::Initialize()
{
    _IsInitialized = false;

    try
    {
        namespace FS = std::filesystem;

        std::vector<std::string> RequiredDirs = {
            GetAnimationsPath(),
            GetCharactersPath(),
            GetMapsPath(),
            GetMonstersPath(),
            GetUIPath(),
            GetItemsPath(),
            GetSoundPath(),
            GetStagesPath()
        };

        for (const auto& DirStr : RequiredDirs)
        {
            FS::path DirPath(DirStr);

            if (FS::exists(DirPath))
            {
                if (!FS::is_directory(DirPath))
                {
                    SafeLog("DataManager::Initialize - path exists but is not a directory: " + DirPath.string());
                    return false;
                }
            }
            else
            {
                if (!FS::create_directories(DirPath))
                {
                    SafeLog("DataManager::Initialize - failed to create directory: " + DirPath.string());
                    return false;
                }
                else
                {
                    SafeLog("Created directory: " + DirPath.string(), ELogImportance::DISPLAY);
                }
            }
        }

        _IsInitialized = true;
        SafeLog("DataManager initialized successfully", ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::Initialize exception: " + std::string(ex.what()));
        _IsInitialized = false;
    }

    return _IsInitialized;
}

// ===== 범용 파일 I/O (폴더 경로를 명시적으로 받음) =====

std::string DataManager::LoadTextFile(const std::string& folderPath, const std::string& fileName)
{
    try
    {
        namespace FS = std::filesystem;
        FS::path filePath = JoinPath(folderPath, fileName);

        if (!FS::exists(filePath) || !FS::is_regular_file(filePath))
        {
            SafeLog("DataManager::LoadTextFile - file not found: " + filePath.string());
            return std::string();
        }

        std::ifstream inputBuf(filePath, std::ios::in);
        if (!inputBuf)
        {
            SafeLog("DataManager::LoadTextFile - failed to open: " + filePath.string());
            return std::string();
        }

        std::ostringstream strStream;
        strStream << inputBuf.rdbuf();
        return strStream.str();
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadTextFile exception: " + std::string(ex.what()));
        return std::string();
    }
}

std::vector<std::vector<std::string>> DataManager::LoadCSVFile(const std::string& folderPath, const std::string& fileName)
{
    std::vector<std::vector<std::string>> result;
    try
    {
        namespace FS = std::filesystem;
        FS::path filePath = JoinPath(folderPath, fileName);

        if (!FS::exists(filePath) || !FS::is_regular_file(filePath))
        {
            SafeLog("DataManager::LoadCSVFile - file not found: " + filePath.string());
            return result;
        }

        std::ifstream inputBuf(filePath);
        if (!inputBuf)
        {
            SafeLog("DataManager::LoadCSVFile - failed to open: " + filePath.string());
            return result;
        }

        auto trim = [](std::string& s) -> void
            {
                const char* whitespace = " \t\n\r";
                auto left = s.find_first_not_of(whitespace);
                if (left == std::string::npos)
                {
                    s.clear();
                    return;
                }
                auto right = s.find_last_not_of(whitespace);
                s = s.substr(left, right - left + 1);
            };

        std::string line;
        while (std::getline(inputBuf, line))
        {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            std::vector<std::string> row;
            std::string field;
            size_t i = 0;
            const size_t n = line.size();

            while (i < n)
            {
                field.clear();

                if (line[i] == '"')
                {
                    ++i;
                    while (i < n)
                    {
                        if (line[i] == '"')
                        {
                            if (i + 1 < n && line[i + 1] == '"')
                            {
                                field.push_back('"');
                                i += 2;
                            }
                            else
                            {
                                ++i;
                                break;
                            }
                        }
                        else
                        {
                            field.push_back(line[i++]);
                        }
                    }
                    while (i < n && line[i] != ',') ++i;
                    if (i < n && line[i] == ',') ++i;
                }
                else
                {
                    while (i < n && line[i] != ',')
                    {
                        field.push_back(line[i++]);
                    }
                    if (i < n && line[i] == ',') ++i;
                }

                trim(field);
                row.push_back(field);
            }

            if (!line.empty() && line.back() == ',')
                row.push_back(std::string());

            result.push_back(std::move(row));
        }
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadCSVFile exception: " + std::string(ex.what()));
    }

    return result;
}

bool DataManager::SaveTextFile(const std::string& folderPath, const std::string& fileName, const std::string& data)
{
    try
    {
        namespace FS = std::filesystem;
        FS::path dirPath(folderPath);
        std::error_code ec;

        if (!FS::exists(dirPath, ec))
        {
            if (!FS::create_directories(dirPath, ec))
            {
                SafeLog("DataManager::SaveTextFile - failed to create directory: " + dirPath.string() + " ec=" + ec.message());
                return false;
            }
        }
        else if (!FS::is_directory(dirPath, ec))
        {
            SafeLog("DataManager::SaveTextFile - path exists but is not directory: " + dirPath.string());
            return false;
        }

        FS::path filePath = dirPath / fileName;

        std::ofstream outBuffer(filePath, std::ios::out | std::ios::trunc);
        if (!outBuffer)
        {
            SafeLog("DataManager::SaveTextFile - failed to open for write: " + filePath.string());
            return false;
        }

        outBuffer << data;
        if (!outBuffer)
        {
            SafeLog("DataManager::SaveTextFile - write failed: " + filePath.string());
            return false;
        }

        outBuffer.flush();
        return true;
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::SaveTextFile exception: " + std::string(ex.what()));
        return false;
    }
}

bool DataManager::DeleteFile(const std::string& folderPath, const std::string& fileName)
{
    namespace FS = std::filesystem;
    std::error_code ec;

    FS::path target = FS::path(folderPath) / fileName;

    if (!FS::exists(target, ec))
    {
        if (ec)
            SafeLog("DataManager::DeleteFile - exists check error: " + ec.message() + " path=" + target.string());
        else
            SafeLog("DataManager::DeleteFile - file not found: " + target.string());
        return false;
    }

    if (!FS::is_regular_file(target, ec))
    {
        if (ec)
            SafeLog("DataManager::DeleteFile - is_regular_file check error: " + ec.message() + " path=" + target.string());
        else
            SafeLog("DataManager::DeleteFile - target is not a regular file: " + target.string());
        return false;
    }

    bool removed = FS::remove(target, ec);
    if (ec)
    {
        SafeLog("DataManager::DeleteFile - remove failed: " + ec.message() + " path=" + target.string());
        return false;
    }

    if (!removed)
    {
        SafeLog("DataManager::DeleteFile - file not removed (unknown reason): " + target.string());
        return false;
    }

    return true;
}

bool DataManager::FileExists(const std::string& folderPath, const std::string& fileName)
{
    namespace FS = std::filesystem;
    std::error_code ec;
    FS::path p = JoinPath(folderPath, fileName);
    return FS::exists(p, ec) && FS::is_regular_file(p, ec);
}

bool DataManager::DirectoryExists(const std::string& dirPath)
{
    namespace FS = std::filesystem;
    std::error_code ec;
    FS::path p(dirPath);
    return FS::exists(p, ec) && FS::is_directory(p, ec);
}

// Item Data를 불러오는 함수
std::vector<ItemData> DataManager::LoadItemData(const std::string& fileName)
{
    std::vector<ItemData> result;

    try
    {
        // Items 폴더에서 CSV 로드
        std::vector<std::vector<std::string>> csv = LoadCSVFile(GetItemsPath(), fileName);

        if (csv.empty())
        {
            SafeLog("DataManager::LoadItemData - CSV file is empty: " + fileName);
            return result;
        }

        // CSV 형식: ItemID,Name,Description,Price,EffectAmount,MaxCount,Stock,AsciiFile,MonsterDropRate,ShopAppearRate
        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 8)
            {
                SafeLog("DataManager::LoadItemData - Invalid row at line " + std::to_string(i) + ": insufficient columns");
                continue;
            }

            try
            {
                ItemData data;
                data.ItemID = row[0];
                data.Name = row[1];
                data.Description = row[2];
                data.Price = std::stoi(row[3]);
                data.EffectAmount = std::stoi(row[4]);
                data.MaxCount = std::stoi(row[5]);
                data.Stock = std::stoi(row[6]);
                data.AsciiFile = row[7];

                // ===== 확장 필드 파싱 (옵션: 없으면 0.0) =====
                if (row.size() > 8 && !row[8].empty()) {
                    data.MonsterDropRate = std::stof(row[8]);
                }
                if (row.size() > 9 && !row[9].empty()) {
                    data.ShopAppearRate = std::stof(row[9]);
                }

                result.push_back(data);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadItemData - Failed to parse row " + std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " items from " + fileName, ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadItemData exception: " + std::string(ex.what()));
    }

    return result;
}

std::vector<MonsterSpawnData> DataManager::LoadMonsterSpawnData(const std::string& fileName)
{
    std::vector<MonsterSpawnData> result;

    try
    {
        // Monsters 폴더에서 CSV 로드
        std::vector<std::vector<std::string>> csv = LoadCSVFile(GetMonstersPath(), fileName);

        if (csv.empty())
        {
            SafeLog("DataManager::LoadMonsterSpawnData - CSV file is empty: " + fileName);
            return result;
        }

        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 13)
            {
                SafeLog("DataManager::LoadMonsterSpawnData - Invalid row at line " + std::to_string(i) + ": insufficient columns");
                continue;
            }

            try
            {
                MonsterSpawnData data;
                data.Enemy_id = row[0];
                data.MonsterName = row[1];
                data.floor = std::stoi(row[2]);
                data.hp = std::stoi(row[3]);
                data.mp = std::stoi(row[4]);
                data.atk = std::stoi(row[5]);
                data.def = std::stoi(row[6]);
                data.dex = std::stoi(row[7]);
                data.luk = std::stoi(row[8]);
                data.crit_rate = std::stod(row[9]);
                data.exp = std::stoi(row[10]);
                data.gold = std::stoi(row[11]);
                data.ascii_file = row[12];

                // attack_name 파싱 (13번째 컬럼, 없으면 기본값 "공격")
                if (row.size() > 13 && !row[13].empty()) {
                    data.attack_name = row[13];
                } else {
                    data.attack_name = "공격";  // 기본값
                }

                result.push_back(data);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadMonsterSpawnData - Failed to parse row " + std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " monsters from " + fileName, ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadMonsterSpawnData exception: " + std::string(ex.what()));
    }

    return result;
}


////랜덤 한개 읽음
//std::tuple<std::string, std::string> DataManager::GetRandomStageAndMonster()
//{
//    auto monsterData = LoadMonsterSpawnData("Monsters.csv");
//    if (monsterData.empty()) return { "", "" };
//
//    // 랜덤 엔진은 게임 매니저에서 가져옴
//
//    // 랜덤 스테이지 선택
//    std::vector<size_t> stageIndices;
//    for (size_t i = 0; i < monsterData.size(); ++i)
//        stageIndices.push_back(i);
//    std::uniform_int_distribution<size_t> dist(0, stageIndices.size() - 1);
//
//    size_t idx = dist(gen);
//
//    // 보스는 제외 (마지막 몬스터)
//    if (idx == monsterData.size() - 1 && monsterData.size() > 1)
//        idx--;
//
//    const auto& selected = monsterData[idx];
//    return { selected.Stage, selected.MonsterName };
//}

// 폴더 내 모든 파일 목록 가져오기
std::vector<std::string> DataManager::GetFilesInDirectory(const std::string& folderPath, const std::string& extension)
{
    std::vector<std::string> fileNames;

    try
    {
        namespace FS = std::filesystem;

        if (!FS::exists(folderPath) || !FS::is_directory(folderPath))
        {
            SafeLog("DataManager::GetFilesInDirectory - Invalid directory: " + folderPath);
            return fileNames;
        }

        // 폴더 내 모든 파일 탐색
        for (const auto& entry : FS::directory_iterator(folderPath))
        {
            if (entry.is_regular_file())
            {
                std::string fileName = entry.path().filename().string();

                // 확장자 필터링 (지정된 경우)
                if (!extension.empty())
                {
                    if (entry.path().extension().string() == extension)
                    {
                        fileNames.push_back(fileName);
                    }
                }
                else
                {
                    fileNames.push_back(fileName);
                }
            }
        }

        // 파일명 정렬 (숫자 순서 보장을 위해)
        std::sort(fileNames.begin(), fileNames.end());

        SafeLog("Found " + std::to_string(fileNames.size()) + " files in " + folderPath,
            ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::GetFilesInDirectory exception: " + std::string(ex.what()));
    }

    return fileNames;
}

//몬스터 읽음
std::optional<MonsterSpawnData> DataManager::GetMonster(const std::string& fileName, int floor)
{
    auto monsterData = LoadMonsterSpawnData(fileName);

    std::vector<MonsterSpawnData> candidates;
    for (const auto& monster : monsterData)
    {
        if (monster.floor == floor)
            candidates.push_back(monster);
    }

    if (candidates.empty())
        return std::nullopt;

    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(gen)];
}

// ===== Stage 데이터 로딩 함수 구현 =====

// 층 정보 로드 (FloorInfo.csv)
std::vector<StageFloorData> DataManager::LoadStageFloorInfo()
{
    std::vector<StageFloorData> result;

    try
    {
        auto csv = LoadCSVFile(GetStagesPath(), "FloorInfo.csv");

        if (csv.size() <= 1)
        {
            SafeLog("DataManager::LoadStageFloorInfo - CSV is empty or header only");
            return result;
        }

        // CSV 형식: Floor,BaseLevel,BattleCount,TotalXP,Description,NormalCount,EliteCount,BossCount
        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 5)
            {
                SafeLog("DataManager::LoadStageFloorInfo - Invalid row at line " + std::to_string(i));
                continue;
            }

            try
            {
                StageFloorData data;
                data.Floor = std::stoi(row[0]);
                data.BaseLevel = std::stoi(row[1]);
                data.BattleCount = std::stoi(row[2]);
                data.TotalXP = std::stoi(row[3]);
                data.Description = row[4];

                // ===== 신규 컬럼 파싱 (옵션: 없으면 0) =====
                if (row.size() > 5) data.NormalCount = row[5].empty() ? 0 : std::stoi(row[5]);
                if (row.size() > 6) data.EliteCount = row[6].empty() ? 0 : std::stoi(row[6]);
                if (row.size() > 7) data.BossCount = row[7].empty() ? 0 : std::stoi(row[7]);

                result.push_back(data);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadStageFloorInfo - Failed to parse row " +
                    std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " floor info entries",
            ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadStageFloorInfo exception: " + std::string(ex.what()));
    }

    return result;
}

// 특정 층의 노드 데이터 로드 (Floor1.csv ~ Floor10.csv)
std::vector<NodeData> DataManager::LoadStageNodes(int floor)
{
    std::vector<NodeData> result;

    try
    {
        std::string fileName = "Floor" + std::to_string(floor) + ".csv";
        auto csv = LoadCSVFile(GetStagesPath(), fileName);

        if (csv.size() <= 1)
        {
            SafeLog("DataManager::LoadStageNodes - CSV is empty or header only: " + fileName);
            return result;
        }

        // CSV 형식: NodeID,NodeType,EnemyType,EnemyCount,EventType,PosX,PosY,Connections
        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 8)
            {
                SafeLog("DataManager::LoadStageNodes - Invalid row at line " +
                    std::to_string(i) + " in " + fileName);
                continue;
            }

            try
            {
                NodeData node;
                node.Id = row[0];

                // NodeType 파싱
                std::string typeStr = row[1];
                if (typeStr == "Start") node.Type = ENodeType::Start;
                else if (typeStr == "Battle") node.Type = ENodeType::Battle;
                else if (typeStr == "Elite") node.Type = ENodeType::Elite;
                else if (typeStr == "Boss") node.Type = ENodeType::Boss;
                else if (typeStr == "Event") node.Type = ENodeType::Event;
                else if (typeStr == "Empty") node.Type = ENodeType::Empty;
                else if (typeStr == "Exit") node.Type = ENodeType::Exit;
                else
                {
                    SafeLog("DataManager::LoadStageNodes - Unknown NodeType: " + typeStr);
                    continue;
                }

                node.EnemyType = row[2];
                node.EnemyCount = row[3].empty() ? 0 : std::stoi(row[3]);
                node.EventType = row[4];
                node.PosX = row[5].empty() ? 0 : std::stoi(row[5]);
                node.PosY = row[6].empty() ? 0 : std::stoi(row[6]);

                // Connections 파싱 (세미콜론으로 구분: "1-1;1-2;1-3")
                std::string connStr = row[7];
                if (!connStr.empty())
                {
                    size_t pos = 0;
                    while ((pos = connStr.find(';')) != std::string::npos)
                    {
                        std::string conn = connStr.substr(0, pos);
                        // Trim whitespace
                        conn.erase(0, conn.find_first_not_of(" \t"));
                        conn.erase(conn.find_last_not_of(" \t") + 1);

                        if (!conn.empty())
                            node.Connections.push_back(conn);

                        connStr.erase(0, pos + 1);
                    }
                    // 마지막 연결 추가
                    connStr.erase(0, connStr.find_first_not_of(" \t"));
                    connStr.erase(connStr.find_last_not_of(" \t") + 1);
                    if (!connStr.empty())
                        node.Connections.push_back(connStr);
                }

                result.push_back(node);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadStageNodes - Failed to parse row " +
                    std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " nodes from " + fileName,
            ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadStageNodes exception: " + std::string(ex.what()));
    }

    return result;
}

// ===== 직업 데이터 로딩 구현 =====

std::vector<ClassData> DataManager::LoadAllClassData(const std::string& fileName)
{
    std::vector<ClassData> result;

    try
    {
        // Characters 폴더에서 CSV 로드
        std::vector<std::vector<std::string>> csv = LoadCSVFile(GetCharactersPath(), fileName);

        if (csv.empty())
        {
            SafeLog("DataManager::LoadAllClassData - CSV file is empty: " + fileName);
            return result;
        }

        // CSV 형식: class_id,role,role_desc,hp,mp,atk,def,dex,luk,crit_rate,ascii_file,ascii_file_select
        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 12)
            {
                SafeLog("DataManager::LoadAllClassData - Invalid row at line " + std::to_string(i) + ": insufficient columns");
                continue;
            }

            try
            {
                ClassData data;
                data._ClassId = row[0];
                data._Role = row[1];
                data._RoleDesc = row[2];
                data._HP = std::stoi(row[3]);
                data._MP = std::stoi(row[4]);
                data._Atk = std::stoi(row[5]);
                data._Def = std::stoi(row[6]);
                data._Dex = std::stoi(row[7]);
                data._Luk = std::stoi(row[8]);
                data._CriticalRate = std::stof(row[9]);
                data._AsciiFile = row[10];
                data._AsciiFileSelect = row[11];

                result.push_back(data);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadAllClassData - Failed to parse row " + std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " classes from " + fileName, ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadAllClassData exception: " + std::string(ex.what()));
    }

    return result;
}

std::optional<ClassData> DataManager::GetClassData(const std::string& classId, const std::string& fileName)
{
    auto allClasses = LoadAllClassData(fileName);

    for (const auto& classData : allClasses)
    {
        if (classData._ClassId == classId)
        {
            return classData;
        }
    }

    SafeLog("DataManager::GetClassData - Class not found: " + classId, ELogImportance::WARNING);
    return std::nullopt;
}

// ===== FloorScaling 데이터 로딩 구현 =====

std::vector<FloorScalingData> DataManager::LoadFloorScaling(const std::string& fileName)
{
    std::vector<FloorScalingData> result;

    try
    {
        // Monsters 폴더에서 CSV 로드
        std::vector<std::vector<std::string>> csv = LoadCSVFile(GetMonstersPath(), fileName);

        if (csv.empty())
        {
            SafeLog("DataManager::LoadFloorScaling - CSV file is empty: " + fileName);
            return result;
        }

        // CSV 형식: floor,hp_mul,mp_mul,atk_mul,def_mul,dex_mul,luk_mul,crit_mul,exp_mul,gold_mul
        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 10)
            {
                SafeLog("DataManager::LoadFloorScaling - Invalid row at line " + std::to_string(i) + ": insufficient columns");
                continue;
            }

            try
            {
                FloorScalingData data;
                data.floor = std::stoi(row[0]);
                data.hp_mul = std::stof(row[1]);
                data.mp_mul = std::stof(row[2]);
                data.atk_mul = std::stof(row[3]);
                data.def_mul = std::stof(row[4]);
                data.dex_mul = std::stof(row[5]);
                data.luk_mul = std::stof(row[6]);
                data.crit_mul = std::stof(row[7]);
                data.exp_mul = std::stof(row[8]);
                data.gold_mul = std::stof(row[9]);

                result.push_back(data);
            }
            catch (const std::exception& ex)
            {
                SafeLog("DataManager::LoadFloorScaling - Failed to parse row " + std::to_string(i) + ": " + std::string(ex.what()));
                continue;
            }
        }

        SafeLog("Loaded " + std::to_string(result.size()) + " floor scaling entries from " + fileName, ELogImportance::DISPLAY);
    }
    catch (const std::exception& ex)
    {
        SafeLog("DataManager::LoadFloorScaling exception: " + std::string(ex.what()));
    }

    return result;
}

std::optional<FloorScalingData> DataManager::GetFloorScaling(int floor, const std::string& fileName)
{
    auto scalingData = LoadFloorScaling(fileName);

    for (const auto& data : scalingData)
    {
        if (data.floor == floor)
        {
            return data;
        }
    }

    SafeLog("DataManager::GetFloorScaling - Floor not found: " + std::to_string(floor), ELogImportance::WARNING);
    return std::nullopt;
}

std::vector<CompanionData> DataManager::LoadCompanionData(const std::string& fileName)
{
    std::vector<CompanionData> companions;
    
    auto csvData = LoadCSVFile(GetCharactersPath(), fileName);
    
    if (csvData.size() <= 1)  // 헤더만 있거나 빈 파일
    {
        return companions;
    }
    
    // 헤더 스킵하고 데이터 파싱
    for (size_t i = 1; i < csvData.size(); ++i)
    {
        const auto& row = csvData[i];
        
        if (row.size() < 12) continue;  // 필드 개수 확인
        
        CompanionData data;
        data._CompanionId = row[0];
        data._Name = row[1];
        data._JobType = row[2];
        data._HP = std::stoi(row[3]);
        data._MP = std::stoi(row[4]);
        data._Atk = std::stoi(row[5]);
        data._Def = std::stoi(row[6]);
        data._Dex = std::stoi(row[7]);
        data._Luk = std::stoi(row[8]);
        data._CritRate = std::stof(row[9]);
        data._AppearRate = std::stof(row[10]);
        data._AsciiFile = row[11];
        
        companions.push_back(data);
    }
    
    return companions;
}

std::optional<CompanionData> DataManager::GetRandomCompanion()
{
    static std::vector<CompanionData> companionCache;
    
    // 캐시가 비어있으면 로드
    if (companionCache.empty())
    {
        companionCache = LoadCompanionData();
        
        if (companionCache.empty())
        {
            return std::nullopt;
        }
    }
    
    // 가중치 기반 랜덤 선택
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    // 누적 확률 계산
    std::vector<float> cumulativeProbabilities;
    float totalProb = 0.0f;
    
    for (const auto& companion : companionCache)
    {
        totalProb += companion._AppearRate;
        cumulativeProbabilities.push_back(totalProb);
    }
    
    // 랜덤 값 생성 (0.0 ~ totalProb)
    std::uniform_real_distribution<float> dist(0.0f, totalProb);
    float randValue = dist(gen);
    
    // 선택된 동료 찾기
    for (size_t i = 0; i < cumulativeProbabilities.size(); ++i)
    {
        if (randValue <= cumulativeProbabilities[i])
        {
            return companionCache[i];
        }
    }
    
    // 실패 시 첫 번째 동료 반환 (안전장치)
    return companionCache[0];
}