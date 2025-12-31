#include "../../include/Manager/DataManager.h"
#include "../../include/Config.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdio>

using namespace std;

static string JoinPath(const string& path1, const string& path2)
{
    // TODO: front Path와 Back Path를 합치는 함수를 구현
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;

    string out = path1;
    if (out.back() != '/' && out.back() != '\\') out.push_back('/');
    if (path2.front() == '/' || path2.front() == '\\') out += path2.substr(1);
    else out += path2;

    return out;
}

string DataManager::GetAsciiPath() const
{
    return JoinPath(string(DEFAULT_RESOURCE_PATH), ASCII_FOLDER);
}

string DataManager::GetCSVPath() const
{
    return JoinPath(string(DEFAULT_RESOURCE_PATH), CSV_FOLDER);
}

string DataManager::GetTextPath() const
{
    return JoinPath(string(DEFAULT_RESOURCE_PATH), TEXT_FOLDER);
}

bool DataManager::Initialize()
{
    // Implementation needed
    // TODO: Save & Load 경로가 유효한지 판단하고 Initialized 상태 설정
    _IsInitialized = false;

    string BasePath = DEFAULT_RESOURCE_PATH;

    try
    {
        namespace FS = std::filesystem;

        vector<string> RequiredDirs = {
            GetAsciiPath(),
            GetCSVPath(),
            GetTextPath()
        };

        for (const auto& DirStr : RequiredDirs)
        {
            FS::path DirPath(DirStr);

            if (FS::exists(DirPath))
            {
                if (!FS::is_directory(DirPath))
                {
                    cerr << "DataManager::Initialize - path exists but is not a directory: " << DirPath.string() << endl;
                    return false;
                }
            }
            else
            {
                // create_directories 에서 문제 발생시 예외 처리
                if (!FS::create_directories(DirPath))
                {
                    cerr << "DataManager::Initialize - failed to create directory: " << DirPath.string() << endl;
                    return false;
                }
            }
        }

        _IsInitialized = true;
    }
    catch (const std::exception& ex)
    {
        cerr << "DataManager::Initialize exception: " << ex.what() << endl;
        _IsInitialized = false;
    }

    return _IsInitialized;
}

string DataManager::LoadTextFile(string FileName)
{
    // TODO: FileName을 받아 텍스트 파일 내용을 문자열로 반환합니다.
    try
    {
        namespace FS = std::filesystem;
        FS::path Path = GetTextPath() + FileName;

        // is_regular_file - 경로가 일반 파일인지 확인하는 함수
        if (!FS::exists(Path) || !FS::is_regular_file(Path))
        {
            cerr << "DataManager::LoadTextFile - file not found: " << Path.string() << endl;
            return string();
        }

        ifstream InputBuf(Path, ios::in);
        if (!InputBuf)
        {
            cerr << "DataManager::LoadTextFile - failed to open: " << Path.string() << endl;
            return string();
        }

        ostringstream StrStream;
        StrStream << InputBuf.rdbuf();
        return StrStream.str();
    }
    catch (const std::exception& ex)
    {
        cerr << "DataManager::LoadTextFile exception: " << ex.what() << endl;
        return string();
    }
}

vector<vector<string>> DataManager::LoadCSVFile(string FileName)
{
    vector<vector<string>> Result;
    try
    {
        namespace FS = std::filesystem;
        FS::path Path = FS::path(GetCSVPath() + FileName);
        if (!FS::exists(Path) || !FS::is_regular_file(Path))
        {
            cerr << "DataManager::LoadCSVFile - file not found: " << Path.string() << endl;
            return Result;
        }

        ifstream InputBuf(Path);
        if (!InputBuf)
        {
            cerr << "DataManager::LoadCSVFile - failed to open: " << Path.string() << endl;
            return Result;
        }

        auto trim = [](string& s) -> void // 공백 처리 람다 함수 선언
            {
                const char* Whitespace = " \t\n\r";          // 공백 문자의 집합(스페이스, 탭, 개행 등)
                auto Left = s.find_first_not_of(Whitespace); // 문자열 왼쪽 앞에서부터 공백이 아닌 문자가 나오는 index
                if (Left == string::npos)
                {
                    s.clear(); return;
                } // 없으면 전체가 공백인 경우에 해당

                auto Right = s.find_last_not_of(Whitespace); // 문자열 오른쪽 뒤에서부터 공백이 아닌 문자가 나오는 index (end())
                s = s.substr(Left, Right - Left + 1);        // 양쪽 공백을 제거한 부분 문자열로 갱신
            };

        string line;
        while (std::getline(InputBuf, line))
        {
            // CR LF 처리
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            vector<string> row;
            string field;
            size_t i = 0;
            const size_t n = line.size();

            while (i < n)
            {
                field.clear();

                if (line[i] == '"')
                {
                    // 따옴표는 스킵합니다
                    ++i;
                    while (i < n)
                    {
                        if (line[i] == '"')
                        {
                            // 공백 문자 "" 처리
                            if (i + 1 < n && line[i + 1] == '"')
                            {
                                field.push_back('"');
                                i += 2;
                            }
                            else
                            { // 닫는 따옴표 처리
                                ++i;
                                break;
                            }
                        }
                        else
                        {
                            // 따욤표가 아닌 경우 추가 
                            field.push_back(line[i++]);
                        }
                    }
                    // 따옴표를 닫고 난 후 콤마를 스킵 처리
                    while (i < n && line[i] != ',') ++i;
                    if (i < n && line[i] == ',') ++i;
                }
                else
                {
                    // 나머지 쓰레기 문자 다 넣고 콤마가 나오면 종료
                    while (i < n && line[i] != ',')
                    {
                        field.push_back(line[i++]);
                    }
                    if (i < n && line[i] == ',') ++i;
                }

                trim(field); // 쓰레기 문자 정리
                row.push_back(field);
            }

            // 콤마로 끝나는 경우 빈 필드를 추가해줌 (e.g., "a,b," 는 -> ["a","b",""])
            if (!line.empty() && line.back() == ',') row.push_back(string());

            // 완전히 빈 줄의 경우 빈 행(크기 0의 벡터)을 유지하게 됨
            Result.push_back(std::move(row));
        }
    }
    catch (const std::exception& ex)
    {
        cerr << "DataManager::LoadCSVFile exception: " << ex.what() << endl;
    }

    return Result;
}

bool DataManager::SaveTextFile(string FileName, string Data)
{
    // TODO: FileName으로 Data 내용을 텍스트 파일로 저장함
    // - GetTextPath() 위치에 있는지 확인 (없다면 생성)
    // - 파일을 텍스트 모드로 열어 덮어쓰기
    // - 쓰기 성공시 true, 실패 시 false를 반환함

    try
    {
        namespace FS = std::filesystem;
        FS::path Dir = FS::path(GetTextPath());
        std::error_code ec;

        // 디렉토리 없으면 생성... 방어코딩
        if (!FS::exists(Dir, ec))
        {
            if (!FS::create_directories(Dir, ec))
            {
                cerr << "DataManager::SaveTextFile - failed to create directory: " << Dir.string()
                    << " ec=" << ec.message() << endl;
                return false;
            }
        }
        else if (!FS::is_directory(Dir, ec))
        {
            cerr << "DataManager::SaveTextFile - path exists but is not directory: " << Dir.string() << endl;
            return false;
        }

        FS::path FilePath = Dir / FileName;

        // ofstream으로 열어서 덮어쓰기 (text mode). 바이너리 모드는 사용하지 않음.
        std::ofstream OutBuffer(FilePath, std::ios::out | std::ios::trunc);
        if (!OutBuffer)
        {
            cerr << "DataManager::SaveTextFile - failed to open for write: " << FilePath.string() << endl;
            return false;
        }

        OutBuffer << Data;
        if (!OutBuffer)
        {
            cerr << "DataManager::SaveTextFile - write failed: " << FilePath.string() << endl;
            return false;
        }

        OutBuffer.flush();
        return true;
    }
    catch (const std::exception& ex)
    {
        cerr << "DataManager::SaveTextFile exception: " << ex.what() << endl;
        return false;
    }

}

bool DataManager::DeleteFile(string FileName)
{
    // TODO: FileName에 해당하는 파일을 삭제하도록 합니다
    // - 파일이 존재하는지 확인하고, 일반 파일인 경우에 삭제를 시도함
    // - 삭제 성공시 true, 실패 시 false를 반환함

    namespace FS = std::filesystem;
    std::error_code ec;

    // 기본적으로 프로젝트의 텍스트 폴더 경로를 기준으로 파일 삭제
    FS::path target = FS::path(GetTextPath()) / FileName;

    // 존재 검사
    if (!FS::exists(target, ec))
    {
        if (ec) cerr << "DataManager::DeleteFile - exists check error: " << ec.message() << " path=" << target.string() << endl;
        else    cerr << "DataManager::DeleteFile - file not found: " << target.string() << endl;
        return false;
    }

    // 일반 파일인지 확인
    if (!FS::is_regular_file(target, ec))
    {
        if (ec) cerr << "DataManager::DeleteFile - is_regular_file check error: " << ec.message() << " path=" << target.string() << endl;
        else    cerr << "DataManager::DeleteFile - target is not a regular file: " << target.string() << endl;
        return false;
    }

    // 삭제 시도
    bool removed = FS::remove(target, ec);
    if (ec)
    {
        cerr << "DataManager::DeleteFile - remove failed: " << ec.message() << " path=" << target.string() << endl;
        return false;
    }

    if (!removed)
    {
        // remove가 false이면서 ec도 비정상 아닌 경우 (예: 이미 삭제됨) 처리
        cerr << "DataManager::DeleteFile - file not removed (unknown reason): " << target.string() << endl;
        return false;
    }

    return true;
}

bool DataManager::FileExists(string FileName)
{
    namespace FS = std::filesystem;
    std::error_code ec;
    FS::path p = GetTextPath() + FileName;
    return FS::exists(p, ec) && FS::is_regular_file(p, ec);
}

bool DataManager::DirectoryExists(const string& DirPath)
{
    namespace FS = std::filesystem;
    std::error_code ec;
    FS::path p = DirPath;
    return FS::exists(p, ec) && FS::is_directory(p, ec);
}