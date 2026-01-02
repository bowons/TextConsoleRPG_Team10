#include "../../include/Manager/DataManager.h"
#include "../../include/Manager/PrintManager.h"
#include "../../include/Item/ItemData.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>

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
            GetItemsPath()
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

        for (size_t i = 1; i < csv.size(); ++i)
        {
            const auto& row = csv[i];

            if (row.empty() || (row.size() == 1 && row[0].empty()))
                continue;

            if (row.size() < 6)
            {
                SafeLog("DataManager::LoadItemData - Invalid row at line " + std::to_string(i) + ": insufficient columns");
                continue;
            }

            try
            {
                ItemData data;
                data.ItemType = row[0];
                data.Name = row[1];
                data.Price = std::stoi(row[2]);
                data.EffectAmount = std::stoi(row[3]);
                data.MaxCount = std::stoi(row[4]);
                data.Stock = std::stoi(row[5]);

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
