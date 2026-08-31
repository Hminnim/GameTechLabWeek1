#include "pch.h"
#include "UResourceManager.h"

UResourceManager::UResourceManager(ID3D11Device* device)
	: m_device(device)
{

}

UResourceManager::~UResourceManager()
{

}

ID3D11ShaderResourceView* UResourceManager::GetTexture(const std::string& key)
{
    // 이미 로드된 텍스처면 바로 반환
    if (m_textures.find(key) != m_textures.end())
        return m_textures[key].Get();

    std::wstring wKey(key.begin(), key.end());

    // 텍스처 생성
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        m_device.Get(),                         
        wKey.c_str(),                           
        nullptr,                                
        m_textures[key].ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        // 로드 실패 시 에러 처리 (예: nullptr 반환 등)
        return nullptr;
    }

    return m_textures[key].Get();
}
