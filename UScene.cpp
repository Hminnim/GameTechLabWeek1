#pragma once
#include "pch.h"
#include "UScene.h"
#include "UPrimitive.h"
#include "UBall.h"
#include "USceneManager.h"
#include "UInputManager.h"
#include "UGameSetting.h"
#include "UMap.h"

void UScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    if (_background)
        _background->Render(renderer);
	if (_map)
		_map->Render(renderer);
    renderer.EndSprite();

    // Game World Object
    for (auto* primitive : _primitives)
        primitive->Render(renderer);

	renderer.BeginSprite();

    // UI
    for (auto* ui : _uis)
        ui->Render(renderer);

    // Button
    for (auto* button : _buttons)
        button->Render(renderer);

	renderer.EndSprite();
}

void UScene::HandleClick(float mouseX, float mouseY)
{
    // Todo: 마우스 클릭시 버튼과 대응되는지 확인

}

void UScene::Update(float Deltatime)
{
    // 공 판정 업데이트
    for (auto* primitive : _primitives)
    {
        if (primitive != nullptr && !primitive->bIsDestroyed)
        {
            primitive->Update(Deltatime, _primitives);
        }        
    }

    // 공 파괴 업데이트
    for (auto it = _primitives.begin(); it != _primitives.end();)
    {
        if ((*it)->bIsDestroyed)
        {
            delete* it;
            it = _primitives.erase(it);         
        }
        else
        {
            ++it;
        }
    }

    UGameManager::GetInstance().Update(_primitives);

    if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON)) {
        float mouseX = UInputManager::GetInstance().GetMousePos().x;
        float mouseY = UInputManager::GetInstance().GetMousePos().y;

        for (int i = _uis.size() - 1; i >= 0; --i) {

            UUI* ui = _uis[i];

            UButton* btn = dynamic_cast<UButton*>(ui);

            if (btn != nullptr && btn->HitTest(mouseX, mouseY)) {
                btn->OnClick();
                break;
            }
        }
    }
}

//////////////////
// UTitleScene  //
//////////////////
void UTitleScene::Initialize()
{
    UUI* temp = new UUI();
    temp->Init("Resources/Title.png", 0, 0, UGameSetting::GetInstance().ScreendWidth, UGameSetting::GetInstance().ScreenHeight);
	AddUI(temp);

    UButton* startbtn = new UButton();
	startbtn->Init("Resources/button_start.png", 400, 700, 400, 400);
    AddUI(startbtn);
    startbtn->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("InGame");
    });

    UButton* exitbtn = new UButton();
	exitbtn->Init("Resources/button_exit.png", 1200, 700, 400, 400);
    AddUI(exitbtn);
    exitbtn->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("GameOver");
    });
}

void UTitleScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);
}

void UTitleScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);
}



//////////////////
// UInGameScene //
//////////////////
void UInGameScene::Initialize()
{
    // 팀별 공 배치 하드 코딩
    std::vector<FVector> RedSpawnPoints = {
        FVector(550.0f, 310.0f, 0.0f),
        FVector(550.0f, 700.0f, 0.0f),
        FVector(550.0f, 1090.0f, 0.0f)
    };
    std::vector<FVector> BlueSpawnPoints = {
        FVector(1485.0f, 310.0f, 0.0f),
        FVector(1485.0f, 700.0f, 0.0f),
        FVector(1485.0f, 1090.0f,0.0f)
    };

    // Ball 소환
    for (FVector& spawnPos : RedSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Red, spawnPos));
    }
    for (FVector& spawnPos : BlueSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Blue, spawnPos));
    }

    UGameManager::GetInstance().InitGame();

    UMap* map = new UMap();
    map->Init("Resources/map.png", 300, 100, UGameSetting::GetInstance().ScreendWidth - 600, UGameSetting::GetInstance().ScreenHeight - 200);
    SetMap(map);

    UUI* background = new UUI();
    background->Init("Resources/background_blue.png", 0, 0, UGameSetting::GetInstance().ScreendWidth, UGameSetting::GetInstance().ScreenHeight);
    SetBackground(background);

}

void UInGameScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);
}

void UInGameScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);
}



////////////////////
// UGameOverScene //
////////////////////
void UGameOverScene::Initialize()
{
    UUI* temp = new UUI();
    temp->Init("Resources/background_red_win.png", 0, 0, UGameSetting::GetInstance().ScreendWidth, UGameSetting::GetInstance().ScreenHeight);
    AddUI(temp);

    UButton* temp2 = new UButton();
    temp2->Init("Resources/button_restart.png", 400, 700, 400, 400);
    AddUI(temp2);
    temp2->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("InGame");
        });

    UButton* temp3 = new UButton();
    temp3->Init("Resources/button_exit.png", 1200, 700, 400, 400);
    AddUI(temp3);
    temp3->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("GameOver");
        });
}

void UGameOverScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);
}

void UGameOverScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);
}
