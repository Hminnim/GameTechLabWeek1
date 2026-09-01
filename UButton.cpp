#pragma once
#include "pch.h"
#include "UButton.h"

bool UButton::HitTest(float mouseX, float mouseY) const
{
    return (mouseX >= _x && mouseX <= _x + _width) &&
        (mouseY >= _y && mouseY <= _y + _height);
}

void UButton::OnClick()
{
    if (_onClick)
    {
        _onClick();
    }
}