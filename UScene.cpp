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
#include "UWall.h"
#include "UEffectManager.h"
#include "UFadeOverlay.h"

void UScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    if (_background)
        _background->Render(renderer);
	if (_map)
		_map->Render(renderer);
    renderer.EndSprite();


    UEffectManager::GetInstance().RenderAuras();
    renderer.DeviceContext->OMSetDepthStencilState(renderer.DefaultDepthStencilState, 0);
    renderer.DeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    renderer.DeviceContext->RSSetState(renderer.RasterizerState);
    renderer.PrepareShader();
    
    // Game World Object
    for (auto* primitive : _primitives)
        primitive->Render(renderer);

	renderer.BeginSprite();
    for (auto* ui : _uis)
        ui->Render(renderer);
    for (auto* button : _buttons)
        button->Render(renderer);
    m_fadeOverlay.Render(renderer);
	renderer.EndSprite();
}

void UScene::HandleClick(float mouseX, float mouseY)
{
    // Todo: 마우스 클릭시 버튼과 대응되는지 확인

}

void UScene::Update(float Deltatime)
{
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
        PostQuitMessage(0);
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

    //int NumBallsPerTeam = 3;

    //float RedStartX = MapMarginX + (MapWidth * 0.15f);
    //float BlueStartX = MapMarginX + (MapWidth * 0.85f);

    //std::vector<FVector> RedSpawnPoints;
    //std::vector<FVector> BlueSpawnPoints;

    //float YInterval = MapHeight / (NumBallsPerTeam + 1);

    //for (int i = 1; i <= NumBallsPerTeam; ++i)
    //{
    //    float SpawnY = MapMarginY + (YInterval * i);
    //    RedSpawnPoints.push_back(FVector(RedStartX, SpawnY, 0.0f));
    //    BlueSpawnPoints.push_back(FVector(BlueStartX, SpawnY, 0.0f));
    //}

    //for (const FVector& spawnPos : RedSpawnPoints)
    //{
    //    AddPrimitive(new UBall("sphere", EPlayer::Red, spawnPos));
    //}
    //for (const FVector& spawnPos : BlueSpawnPoints)
    //{
    //    AddPrimitive(new UBall("sphere", EPlayer::Blue, spawnPos));
    //}

    float BtnWidth = ScreenWidth * (120.0f / 2040.0f);
    float BtnHeight = BtnWidth;
    float BtnX = ScreenWidth * (92.0f / 2040.0f);
    float RightBtnX = ScreenWidth - BtnX - BtnWidth;
    int NumButtons = 5;
    float BtnYInterval = ScreenHeight / (NumButtons + 1);
    // 화면 크기에 따른 보정 ----------------------------------------------
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
        skillBtn->SetOnClick([this,slotInfo]() {            
            this->PlayerController.ApplySkill(slotInfo.AssignedSkill);
            });

        // skillBtn->SetSlot(currentSlot); 

        AddSkillButton(skillBtn);
    }


    UGameManager::GetInstance().InitGame();
}

void UInGameScene::Update(float deltaTime)
{
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

    // Skill Button 업데이트
    for (USkillButton* btn : m_skillButtons)
    {
        btn->Update(deltaTime);
    }

    if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON) && PlayerController.bHasSelectedBall()) {
        LONG mouseX = UInputManager::GetInstance().GetMousePos().x;
        LONG mouseY = UInputManager::GetInstance().GetMousePos().y;

        for (USkillButton* btn : m_skillButtons)
        {
            if (btn != nullptr && btn->HitTest((float)mouseX, (float)mouseY)) {
                btn->OnClick();
                break;
            }
        }
    }
    

    UScene::Update(deltaTime);

    // 공 발사
    PlayerController.Update(_primitives);

    // 공 판정 업데이트
    for (auto* primitive : _primitives)
    {
        if (primitive != nullptr && !primitive->bIsDestroyed)
        {
            primitive->Update(deltaTime, _primitives);
        }
    }

    if (!_pendingPrimitives.empty())
    {
        for (auto* p : _pendingPrimitives)
        {
            _primitives.push_back(p);
        }
        _pendingPrimitives.clear(); // 대기실 비우기
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

    UGameManager::GetInstance().Update(_primitives,deltaTime);
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

void UInGameScene::Enter()
{
    // 공 소환 전 기존에 있던 공 제거
    for (auto it = _primitives.begin(); it != _primitives.end();)
    {
        delete* it;
        it = _primitives.erase(it);
    }

    // 스킬들 리셋
	UGameManager::GetInstance().ResetSkiils();

    // 지정된 위치 공 소환
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    float MapMarginX = UGameSetting::GetInstance().MapMarginX;
    float MapMarginY = UGameSetting::GetInstance().MapMarginY;
    float MapWidth = ScreenWidth - (MapMarginX * 2);
    float MapHeight = ScreenHeight - (MapMarginY * 2);

    int NumBallsPerTeam = UGameSetting::GetInstance().BallsPerTeam;

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

    for (const FVector& spawnPos : RedSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Red, spawnPos));
    }
    for (const FVector& spawnPos : BlueSpawnPoints)
    {
        AddPrimitive(new UBall("sphere", EPlayer::Blue, spawnPos));
    }
    UGameManager::GetInstance().InitGame();
}



////////////////////
// UGameOverScene //
////////////////////
void UGameOverScene::Initialize()
{
    // 화면 크기에 따른 보정 ----------------------------------------------
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    UUI* backgroundRedWin = new UUI();
    UUI* backgroundBlueWin = new UUI();
    UUI* backgroundDraw = new UUI();
    backgroundRedWin->Init("Resources/background_red_win.png", 0, 0, ScreenWidth, ScreenHeight);
    backgroundBlueWin->Init("Resources/background_blue_win.png", 0, 0, ScreenWidth, ScreenHeight);
    backgroundDraw->Init("Resources/background_draw.png", 0, 0, ScreenWidth, ScreenHeight); // 임시 이미지
    SetBackground(backgroundRedWin, backgroundBlueWin, backgroundDraw);

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
        PostQuitMessage(0);
        });
}

void UGameOverScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);
}

void UGameOverScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    _resultBackgrounds[UGameManager::GetInstance().CurrentGameResult]->Render(renderer);
    renderer.EndSprite();

    UScene::Render(renderer);
}

void UGameOverScene::Enter()
{

}



/////////////////
// UDraftScene //
/////////////////
void DraftScene::Initialize()
{
}

void DraftScene::Update(float deltaTime)
{
}

void DraftScene::Render(URenderer& renderer)
{
}

void DraftScene::Enter()
{
}

/////////////////
// UIntroScene //
/////////////////
void UIntroScene::Initialize()
{
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    UUI* background = new UUI();
    background->Init("Resources/background_intro.png", 0, 0, ScreenWidth, ScreenHeight);
    SetBackground(background);
    
    float iconSize = 300.0f;
    float startX = (ScreenWidth / 2.0f) - (iconSize / 2.0f);
    float yOffset = 200.0f;
    float startY = (ScreenHeight / 2.0f) - (iconSize / 2.0f) - yOffset;

    UUI* out = new UUI();
    out->Init("Resources/Jungle_out.png", startX, startY, iconSize, iconSize);
    AddOut(out);

    UUI* in = new UUI();
    in->Init("Resources/Jungle_in.png", startX, startY, iconSize, iconSize);
    AddIn(in);
}

void UIntroScene::Update(float deltaTime)
{
    m_elapsedTime += deltaTime;
    char buf[128];
    sprintf_s(buf, "dt=%f, elapsed=%f, renderTime=%f\n", deltaTime, m_elapsedTime, m_renderTime);
    OutputDebugStringA(buf);
    if (m_elapsedTime >= m_renderTime) {
        USceneManager::GetInstance().RequestChangeScene("Title");
    }

    float rotateSpeed = 1.0f;
    m_rotationAngle += rotateSpeed * deltaTime;

    if (m_in) {
        m_in->SetRotation(m_rotationAngle);
    }
}

void UIntroScene::Render(URenderer& renderer)
{
    renderer.BeginSprite();
    m_background->Render(renderer);
    m_out->Render(renderer);
    m_in->Render(renderer);
    renderer.EndSprite();
}

void UIntroScene::Enter()
{
}
