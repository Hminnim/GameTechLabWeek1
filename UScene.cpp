#pragma once
#include "pch.h"
#include "UScene.h"
#include "UPrimitive.h"
#include "UBall.h"

void UScene::Render(URenderer& renderer)
{
    // Game World Object
    for (auto* primitive : _primitives)
        primitive->Render(renderer);

	renderer.m_spriteBatch->Begin();

    // UI
    for (auto* ui : _uis)
        ui->Render(renderer);

    // Button
    for (auto* button : _buttons)
        button->Render(renderer);

	renderer.m_spriteBatch->End();
}

void UScene::HandleClick(float mouseX, float mouseY)
{
    // Todo: 마우스 클릭시 버튼과 대응되는지 확인

}



//////////////////
// UTitleScene  //
//////////////////
void UTitleScene::Initialize()
{
	AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));
    AddPrimitive(new UBall("sphere", "Resources/test.png"));

    UUI* temp = new UUI();
    temp->Init("Resources/Title.png", 0, 0, 2040, 1400);
	AddUI(temp);

	UUI* temp2 = new UButton();
	temp2->Init("Resources/button_start.png", 400, 700, 400, 400);
    AddUI(temp2);

	UUI* temp3 = new UButton();
	temp3->Init("Resources/button_exit.png", 1200, 700, 400, 400);
    AddUI(temp3);
}

void UTitleScene::Update(float deltaTime)
{
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
}

void UInGameScene::Update(float deltaTime)
{
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
}

void UGameOverScene::Update(float deltaTime)
{
}

void UGameOverScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);
}
