#pragma once
#include "pch.h"
#include "UButton.h"
#include "URenderer.h"
#include "UResourceManager.h"

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
	if (!_usedTextureKey.empty())
	{
		_textureKey = _usedTextureKey;
		_isUsed = true;
	}
}

void UButton::Render(URenderer& renderer)
{
	//if (!_isActive || !_srv)
	//	return;
    if (!_isUsed)
    {
		UUI::Render(renderer);
    }
    else
    {
        _srv = UResourceManager::GetInstance().GetTexture(_usedTextureKey);
        RECT destRect = {
            (LONG)_x,
            (LONG)_y,
            (LONG)(_x + _width),
            (LONG)(_y + _height)
        };
        renderer.m_spriteBatch->Draw(_srv, destRect);
    }
}
