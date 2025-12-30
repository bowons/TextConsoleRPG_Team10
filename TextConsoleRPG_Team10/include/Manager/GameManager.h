#pragma once
#include "../Singleton.h"

#include <memory>

using namespace std;

class Player;

class GameManager : public Singleton<GameManager>
{
private:
    shared_ptr<Player> _MainPlayer;
    bool _IsGameOver = false;

public:
    void StartGame();
    void RunMainLoop();
    void EndGame();
};
