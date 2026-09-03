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

    // 1. 텍스처의 원본 해상도를 알아내서 회전 중심점(Origin) 구하기
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    _srv->GetResource(resource.GetAddressOf());

    Microsoft::WRL::ComPtr<ID3D11Texture2D> tex2D;
    resource.As(&tex2D);

    D3D11_TEXTURE2D_DESC desc;
    tex2D->GetDesc(&desc);

    // Origin은 텍스처 픽셀 기준의 정중앙입니다.
    DirectX::XMFLOAT2 origin(desc.Width / 2.0f, desc.Height / 2.0f);

    // 2. 그려질 화면 중심 좌표 계산
    LONG centerX = (LONG)(_x + _width / 2.0f);
    LONG centerY = (LONG)(_y + _height / 2.0f);

    // SpriteBatch에 Origin을 넘기면, destRect의 좌측상단(left, top) 위치에 Origin이 오게 됩니다.
    // 폭과 높이는 (우측-좌측), (하단-상단)으로 정상 적용됩니다.
    RECT destRect = {
        centerX,                   // 화면에 찍힐 중심 X 좌표
        centerY,                   // 화면에 찍힐 중심 Y 좌표
        (LONG)(centerX + _width),
        (LONG)(centerY + _height)
    };

    // 3. 회전을 지원하는 Draw 오버로딩 사용
    renderer.m_spriteBatch->Draw(
        _srv,
        destRect,
        nullptr,                      // sourceRect (nullptr이면 전체 이미지)
        DirectX::Colors::White,       // 색상 (기본 흰색)
        _rotation,                    // 회전값 (라디안)
        origin                        // 회전 중심점
    );
}