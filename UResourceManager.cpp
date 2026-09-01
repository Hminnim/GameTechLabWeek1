#include "pch.h"
#include "UResourceManager.h"

UResourceManager& UResourceManager::GetInstance()
{
    static UResourceManager instance;
    return instance;
}

void UResourceManager::Initialize(ID3D11Device* device, ID3D11Buffer* vertexBuffer, UINT numVertices)
{
    m_device = device;
    m_vertexBuffers["sphere"] = vertexBuffer;
    m_numVertices = numVertices;
}

ID3D11ShaderResourceView* UResourceManager::GetTexture(const std::string& key)
{
    // 이미 로드된 텍스처면 바로 반환
    if (m_textures.find(key) != m_textures.end())
        return m_textures[key].Get();

    std::wstring wKey(key.begin(), key.end());
    OutputDebugStringW(wKey.c_str());
    // 텍스처 생성
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),                         
        wKey.c_str(),                           
        nullptr,                                
        m_textures[key].ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        _com_error err(hr);

        OutputDebugStringW(err.ErrorMessage());
        return nullptr;
    }

    return m_textures[key].Get();
}

ID3D11Buffer* UResourceManager::GetVertexBuffer(const std::string& key)
{
    return m_vertexBuffers[key].Get();
}

UINT UResourceManager::GetNumVertices(const std::string& key)
{
    return m_numVertices;
}
