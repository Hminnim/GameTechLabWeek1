#pragma once

#include "pch.h"

class UResourceManager
{
public:
	static UResourceManager& GetInstance();

	void Initialize(ID3D11Device* device);

	UResourceManager(const UResourceManager&) = delete;
	UResourceManager& operator=(const UResourceManager&) = delete;

	ID3D11ShaderResourceView* GetTexture(const std::string& key);

private:
	UResourceManager() = default;
	~UResourceManager() = default;
private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
};

