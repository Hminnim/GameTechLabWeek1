#pragma once
#include "pch.h"
#include "UUI.h"
#include "URenderer.h"
#include "UResourceManager.h"

// Initialization
bool UUI::Init(const std::string& texturePath, float x, float y, float width, float height)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;

    _textureKey = texturePath;

    // Todo: 텍스처 로드

    FVertexSimple vertices[4] =
    {
        { x,         y,          0.0f, 0.0f, 0.0f }, // left-top
        { x + width, y,          0.0f, 1.0f, 0.0f }, // right-top
        { x,         y + height, 0.0f, 0.0f, 1.0f }, // left-bottom
        { x + width, y + height, 0.0f, 1.0f, 1.0f }, // right-bottom
    };

    //_mesh = renderer.CreateVertexBuffer(vertices, sizeof(FVertexSimple)*4);

    return _textureKey.empty();
}

// Rendering
void UUI::Render(URenderer& renderer)
{
    // render 대상 아니거나 srv, mesh 없으면 render pass
    //if (!_isActive || !_srv || !_mesh)
    //    return;

    // Todo: 렌더링 구현
    _srv = UResourceManager::GetInstance().GetTexture(_textureKey);
    RECT destRect = {
        (LONG)_x,
        (LONG)_y,
        (LONG)(_x + _width),
        (LONG)(_y + _height)
    };
    renderer.m_spriteBatch->Draw(_srv, destRect);
}