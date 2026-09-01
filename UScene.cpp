#pragma once
#include "pch.h"
#include "UScene.h"
#include "UPrimitive.h"

void UScene::Render(URenderer& renderer)
{
    // Game World Object
    for (auto* primitive : _primitives)
        primitive->Render(renderer);

    // UI
    for (auto* ui : _uis)
        ui->Render(renderer);

    // Button
    for (auto* button : _buttons)
        button->Render(renderer);
}

void UScene::HandleClick(float mouseX, float mouseY)
{
    // Todo: 마우스 클릭시 버튼과 대응되는지 확인

}
