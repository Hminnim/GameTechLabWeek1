#pragma once
#include "pch.h"
#include "UButton.h"

bool UButton::HitTest(float mouseX, float mouseY) const
{
    // Todo: 클릭 여부 판정
    return false;
}

void UButton::OnClick()
{
    if (_onClick)
    {
        _onClick();
    }
}