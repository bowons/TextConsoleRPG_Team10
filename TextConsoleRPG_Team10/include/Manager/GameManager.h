#pragma once
#include "../Singleton.h"

#include <memory>
#include <random>

static std::mt19937 gen(std::random_device{}());

class Player;

class GameManager : public Singleton<GameManager>
{
private:
    std::shared_ptr<Player> _MainPlayer;
    bool _IsGameOver = false;

private:
    GameManager() = default;
    friend class Singleton<GameManager>;

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

public:
    void StartGame();
    void RunMainLoop();
    void EndGame();
};
