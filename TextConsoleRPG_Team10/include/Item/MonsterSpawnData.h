#pragma once
#include <string>

struct MonsterSpawnData
{
    std::string Enemy_id;
    std::string MonsterName;
    int floor;
    int hp;
    int mp;
    int atk;
    int def;
    int dex;
    int luk;
    double crit_rate;
    int exp;
    int gold;
    std::string ascii_file;
    std::string attack_name;  // 몬스터 기본 공격명
};