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
#include "UButton.h"

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

	// GameManager 업데이트
    UGameManager::GetInstance().Update(_primitives);

    // UI 업데이트
    for (auto* ui : _uis)
    {
        if (ui != nullptr && ui->IsActive())
        {
            ui->Update(Deltatime);
        }
    }

    // Button 업데이트
    if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON)) {
        LONG mouseX = UInputManager::GetInstance().GetMousePos().x;
        LONG mouseY = UInputManager::GetInstance().GetMousePos().y;

        for (int i = (int)_uis.size() - 1; i >= 0; --i) {

            UUI* ui = _uis[i];

            UButton* btn = dynamic_cast<UButton*>(ui);

            if (btn != nullptr && btn->HitTest((float)mouseX, (float)mouseY)) {
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
        USceneManager::GetInstance().RequestChangeScene("InGame");
        });

    UButton* exitbtn = new UButton();
    exitbtn->Init("Resources/button_exit.png", ExitBtnX, ButtonY, ButtonWidth, ButtonHeight);
    AddUI(exitbtn);
    exitbtn->SetOnClick([]() {
        USceneManager::GetInstance().RequestChangeScene("GameOver");
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

    float RedStartX = MapMarginX + (MapWidth * 0.85f);
    float BlueStartX = MapMarginX + (MapWidth * 0.15f);

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
    float RightBtnX = ScreenWidth - BtnX - BtnWidth;
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


    // Load&Set Skill Button

	// Set Skill Button Positions
    for (int i = 0; i < (int)ESlot::MaxCount; ++i)
    {
        int yIndex = (i % 5) + 1;
        float slotY = (BtnYInterval * yIndex) - (BtnHeight * 0.5f);
        float slotX = (i < (int)ESlot::MaxCount / 2) ? BtnX : RightBtnX;
        m_slotData[(ESlot)i] = { (ESkillType)((i % 5) + 1),slotX, slotY };
    }

    std::string skillNames[] = {
    "none",     // 0: None
    "freeze",   // 1: Freeze
    "giant",    // 2: Giant
    "heavier",  // 3: Heavier
    "mine",     // 4: Mine
    "repulse"   // 5: Repulse
    };

    for (int i = 0; i < (int)ESlot::MaxCount; ++i)
    {
        ESlot currentSlot = (ESlot)i;
        FSlotData slotInfo = m_slotData[currentSlot];

        std::string baseName = skillNames[(int)slotInfo.AssignedSkill];

        std::string normalTex = "Resources/button_" + baseName + ".png";
        std::string usedTex = "Resources/button_" + baseName + "_used.png";

        USkillButton* skillBtn = new USkillButton();

        skillBtn->Init(normalTex, slotInfo.x, slotInfo.y, BtnWidth, BtnHeight);
        skillBtn->SetUsedTexture(usedTex);
        skillBtn->SetSkillType(slotInfo.AssignedSkill);

        // skillBtn->SetSlot(currentSlot); 

        AddSkillButton(skillBtn);
    }


    UGameManager::GetInstance().InitGame();
}

void UInGameScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);

    bool isRedTurn = (UGameManager::GetInstance().CurrentPlayerTurn == EPlayer::Red);

    int halfCount = (int)ESlot::MaxCount / 2;

    for (int i = 0; i < m_skillButtons.size(); ++i)
    {
        if (m_skillButtons[i] != nullptr)
        {
            bool bActive = isRedTurn ? (i >= halfCount) : (i < halfCount);

            m_skillButtons[i]->SetActive(bActive);
        }
    }
}

void UInGameScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    m_backgrounds[UGameManager::GetInstance().CurrentPlayerTurn]->Render(renderer);
    renderer.EndSprite();
    UScene::Render(renderer);
    renderer.BeginSprite();
    for (const auto& skillButton : m_skillButtons)
	{
		skillButton->Render(renderer);
	}
    renderer.EndSprite();
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
        USceneManager::GetInstance().RequestChangeScene("InGame");
        });

    UButton* temp3 = new UButton();
    temp3->Init("Resources/button_exit.png", ExitBtnX, ButtonY, ButtonWidth, ButtonHeight);
    AddUI(temp3);
    temp3->SetOnClick([]() {
        USceneManager::GetInstance().RequestChangeScene("GameOver");
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
