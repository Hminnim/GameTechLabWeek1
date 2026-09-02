#pragma once
#include "pch.h"
#include "UScene.h"
#include "UPrimitive.h"
#include "UBall.h"
#include "USceneManager.h"
#include "UInputManager.h"
#include "UGameSetting.h"
#include "UMap.h"
#include "UGameManager.h"

void UScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    if (_background)
        _background->Render(renderer);
	if (_map)
		_map->Render(renderer);
    renderer.EndSprite();

    for (auto* primitive : _primitives)
        primitive->Render(renderer);

	renderer.BeginSprite();
    for (auto* ui : _uis)
        ui->Render(renderer);
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
    // 화면 크기에 따른 보정 ----------------------------------------------
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    UUI* temp = new UUI();
    temp->Init("Resources/Title.png", 0, 0, ScreenWidth, ScreenHeight);
    AddUI(temp);

    float ButtonWidth = 400.0f;
    float ButtonHeight = 400.0f;

    float ButtonY = ScreenHeight * 0.50f;

    float ButtonGap = 400.0f;
    float TotalWidth = (ButtonWidth * 2) + ButtonGap;

    float StartBtnX = (ScreenWidth - TotalWidth) / 2.0f;
    float ExitBtnX = StartBtnX + ButtonWidth + ButtonGap;
    // 화면 크기에 따른 보정 ----------------------------------------------

    UButton* startbtn = new UButton();
    startbtn->Init("Resources/button_start.png", StartBtnX, ButtonY, ButtonWidth, ButtonHeight);
    AddUI(startbtn);
    startbtn->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("InGame");
        });

    UButton* exitbtn = new UButton();
    exitbtn->Init("Resources/button_exit.png", ExitBtnX, ButtonY, ButtonWidth, ButtonHeight);
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
    // 화면 크기에 따른 보정 ----------------------------------------------
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    float MapMarginX = 300.0f;
    float MapMarginY = 100.0f;
    float MapWidth = ScreenWidth - (MapMarginX * 2);
    float MapHeight = ScreenHeight - (MapMarginY * 2);

    int NumBallsPerTeam = 3;

    float RedStartX = MapMarginX + (MapWidth * 0.15f);
    float BlueStartX = MapMarginX + (MapWidth * 0.85f);

    std::vector<FVector> RedSpawnPoints;
    std::vector<FVector> BlueSpawnPoints;

    float YInterval = MapHeight / (NumBallsPerTeam + 1);

    for (int i = 1; i <= NumBallsPerTeam; ++i)
    {
        float SpawnY = MapMarginY + (YInterval * i);
        RedSpawnPoints.push_back(FVector(RedStartX, SpawnY, 0.0f));
        BlueSpawnPoints.push_back(FVector(BlueStartX, SpawnY, 0.0f));
    }

    float BtnWidth = ScreenWidth * (120.0f / 2040.0f);
    float BtnHeight = BtnWidth;
    float BtnX = ScreenWidth * (92.0f / 2040.0f);
    int NumButtons = 5;
    float BtnYInterval = ScreenHeight / (NumButtons + 1);
    // 화면 크기에 따른 보정 ----------------------------------------------

    for (const FVector& spawnPos : RedSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Red, spawnPos));
    }
    for (const FVector& spawnPos : BlueSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Blue, spawnPos));
    }

    UMap* map = new UMap();
    map->Init("Resources/map.png", MapMarginX, MapMarginY, MapWidth, MapHeight);
    SetMap(map);

    UUI* backgroundRed = new UUI();
	UUI* backgroundBlue = new UUI();
    backgroundRed->Init("Resources/background_red.png", 0, 0, ScreenWidth, ScreenHeight);
    backgroundBlue->Init("Resources/background_blue.png", 0, 0, ScreenWidth, ScreenHeight);
    SetBackground(backgroundBlue, backgroundRed);

    UButton* freezeBtn = new UButton();
    float Y1 = (BtnYInterval * 1) - (BtnHeight * 0.5f);
    freezeBtn->Init("Resources/button_freeze.png", BtnX, Y1, BtnWidth, BtnHeight);
    freezeBtn->SetUsedTexture("Resources/button_freeze_used.png");
    AddUI(freezeBtn);

    UButton* giantBtn = new UButton();
    float Y2 = (BtnYInterval * 2) - (BtnHeight * 0.5f);
    giantBtn->Init("Resources/button_giant.png", BtnX, Y2, BtnWidth, BtnHeight);
    giantBtn->SetUsedTexture("Resources/button_giant_used.png");
    AddUI(giantBtn);

    UButton* heavierBtn = new UButton();
    float Y3 = (BtnYInterval * 3) - (BtnHeight * 0.5f);
    heavierBtn->Init("Resources/button_heavier.png", BtnX, Y3, BtnWidth, BtnHeight);
    heavierBtn->SetUsedTexture("Resources/button_heavier_used.png");
    AddUI(heavierBtn);

    UButton* mineBtn = new UButton();
    float Y4 = (BtnYInterval * 4) - (BtnHeight * 0.5f);
    mineBtn->Init("Resources/button_mine.png", BtnX, Y4, BtnWidth, BtnHeight);
    mineBtn->SetUsedTexture("Resources/button_mine_used.png");
    AddUI(mineBtn);

    UButton* repulseBtn = new UButton();
    float Y5 = (BtnYInterval * 5) - (BtnHeight * 0.5f);
    repulseBtn->Init("Resources/button_repulse.png", BtnX, Y5, BtnWidth, BtnHeight);
    repulseBtn->SetUsedTexture("Resources/button_repulse_used.png");
    AddUI(repulseBtn);

    UGameManager::GetInstance().InitGame();
}

void UInGameScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);
}

void UInGameScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    _backgrounds[UGameManager::GetInstance().CurrentPlayerTurn]->Render(renderer);
    renderer.EndSprite();
    UScene::Render(renderer);
}



////////////////////
// UGameOverScene //
////////////////////
void UGameOverScene::Initialize()
{
    // 화면 크기에 따른 보정 ----------------------------------------------
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    UUI* temp = new UUI();
    temp->Init("Resources/background_red_win.png", 0, 0, ScreenWidth, ScreenHeight);
    AddUI(temp);

    float ButtonWidth = 400.0f;
    float ButtonHeight = 400.0f;
    float ButtonY = ScreenHeight * 0.50f;

    float ButtonGap = 400.0f;
    float TotalWidth = (ButtonWidth * 2) + ButtonGap;

    float RestartBtnX = (ScreenWidth - TotalWidth) / 2.0f;
    float ExitBtnX = RestartBtnX + ButtonWidth + ButtonGap;
    // 화면 크기에 따른 보정 ----------------------------------------------

    UButton* temp2 = new UButton();
    temp2->Init("Resources/button_restart.png", RestartBtnX, ButtonY, ButtonWidth, ButtonHeight);
    AddUI(temp2);
    temp2->SetOnClick([]() {
        USceneManager::GetInstance().ChangeScene("InGame");
        });

    UButton* temp3 = new UButton();
    temp3->Init("Resources/button_exit.png", ExitBtnX, ButtonY, ButtonWidth, ButtonHeight);
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
