#pragma once
#include "pch.h"
#include "UUI.h"
#include "URenderer.h"

// Initialization
bool UUI::Init(URenderer& renderer, const std::wstring& texturePath, float x, float y, float width, float height)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;

    // Todo: 텍스처 로드

    FVertexSimple vertices[4] =
    {
        { x,         y,          0.0f, 0.0f, 0.0f }, // left-top
        { x + width, y,          0.0f, 1.0f, 0.0f }, // right-top
        { x,         y + height, 0.0f, 0.0f, 1.0f }, // left-bottom
        { x + width, y + height, 0.0f, 1.0f, 1.0f }, // right-bottom
    };

    _mesh = renderer.CreateVertexBuffer(vertices, sizeof(FVertexSimple)*4);
    return _mesh != nullptr;

}

// Rendering
void UUI::Render(URenderer& renderer)
{
    // render 대상 아니거나 srv, mesh 없으면 render pass
    if (!_isActive || !_srv || !_mesh)
        return;

    // Todo: 렌더링 구현

}