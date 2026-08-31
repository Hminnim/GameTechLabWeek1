#pragma once

#include "pch.h"

class UResourceManager
{
public:
	UResourceManager(ID3D11Device* device);
	~UResourceManager();

	ID3D11ShaderResourceView* GetTexture(const std::string& key);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
};

