#include "pch.h"
#include "UUI.h"

extern ID3D11BlendState* g_alphaBlendState;

struct UIVertex
{
    float x, y, z; // 화면 위치
    float u, v; // 텍스처 좌표
};

// Initialization
bool UUI::Init(ID3D11Device* device, const std::wstring& texturePath, float x, float y, float width, float height)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;

    UIVertex vertices[4] =
    {
        { x,         y,          0.0f, 0.0f, 0.0f }, // left-top
        { x + width, y,          0.0f, 1.0f, 0.0f }, // right-top
        { x,         y + height, 0.0f, 0.0f, 1.0f }, // left-bottom
        { x + width, y + height, 0.0f, 1.0f, 1.0f }, // right-bottom
    };

    _stride = sizeof(UIVertex);
    _offset = 0;
    return true;
}

// Render
void UUI::Render(ID3D11DeviceContext* context)
{
    // render 대상 아니거나 srv, mesh 없으면 render pass
    if (!_isActive || !_srv || !_mesh)
        return;

    context->OMSetBlendState(g_alphaBlendState, nullptr, 0xffffffff);
    context->PSSetShaderResources(0, 1, &_srv);
    context->IASetVertexBuffers(0, 1, &_mesh, &_stride, &_offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    context->Draw(4, 0);
}