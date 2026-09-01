#pragma once
#include "pch.h"
#include "UScene.h"
#include "UPrimitive.h"
#include "UBall.h"
#include "USceneManager.h"
#include "UInputManager.h"


void UScene::Render(URenderer& renderer)
{
	renderer.BeginSprite();

    // UI
    for (auto* ui : _uis)
        ui->Render(renderer);

    // Button
    for (auto* button : _buttons)
        button->Render(renderer);

	renderer.EndSprite();

    // Game World Object
    for (auto* primitive : _primitives)
        primitive->Render(renderer);
}

void UScene::HandleClick(float mouseX, float mouseY)
{
    // Todo: 마우스 클릭시 버튼과 대응되는지 확인

}

void UScene::Update(float Deltatime)
{
    for (auto* primitive : _primitives)
    {
        primitive->Update(Deltatime, _primitives);
    }

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
    temp->Init("Resources/Title.png", 0, 0, 2040, 1400);
	AddUI(temp);

    UButton* temp2 = new UButton();
	temp2->Init("Resources/button_start.png", 400, 700, 400, 400);
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
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));

    UUI* temp = new UUI();
    temp->Init("Resources/background_blue.png", 0, 0, 2040, 1400);
    AddUI(temp);
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
    temp->Init("Resources/background_red_win.png", 0, 0, 2040, 1400);
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
