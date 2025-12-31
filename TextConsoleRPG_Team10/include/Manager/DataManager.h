#pragma once
#include "../Singleton.h"
#include <string>
#include <vector>

using namespace std;

class Player;

class DataManager : public Singleton<DataManager>
{
private:
    bool _IsInitialized;

private:
    DataManager() = default;
    friend class Singleton<DataManager>;

    DataManager(const DataManager&) = delete;
    DataManager& operator=(const DataManager&) = delete;

    string GetAsciiPath() const;
    string GetCSVPath() const;
    string GetTextPath() const;

public:
    bool Initialize();
    string LoadTextFile(string fileName);
    vector<vector<string>> LoadCSVFile(string fileName);
    bool SaveTextFile(string fileName, string data);
    bool FileExists(string fileName);
    bool DeleteFile(string fileName);
    bool DirectoryExists(const string& DirPath);

    inline const bool IsInitialized() { return _IsInitialized; };
};
