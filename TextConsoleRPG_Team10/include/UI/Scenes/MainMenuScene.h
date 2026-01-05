#pragma once
#include "../UIScene.h"

class MainMenuScene : public UIScene
{
    public:
    MainMenuScene();
    ~MainMenuScene() override;

    void Enter() override;
    void Exit() override;
    void Update() override;
    void Render() override;
    void HandleInput() override;
};
