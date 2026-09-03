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
void UUI::Render(URenderer& renderer) {
    _srv = UResourceManager::GetInstance().GetTexture(_textureKey);
    if (!_srv) return;

    if (!_isActive) return;

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    _srv->GetResource(resource.GetAddressOf());

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
    resource.As(&tex2D);

    D3D11_TEXTURE2D_DESC desc;
    tex2D->GetDesc(&desc);

    DirectX::XMFLOAT2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

    LONG centerX = (LONG)(_x + _width / 2.0f);
    LONG centerY = (LONG)(_y + _height / 2.0f);

    RECT destRect = {
        centerX,                   
        centerY,                   
        (LONG)(centerX + _width),
        (LONG)(centerY + _height)
    };

    renderer.m_spriteBatch->Draw(
        _srv,
        destRect,
        nullptr,                      // sourceRect (nullptr이면 전체 이미지)
        DirectX::Colors::White,       // 색상 (기본 흰색)
        _rotation,                    // 회전값 (라디안)
        origin                        // 회전 중심점
    );
}