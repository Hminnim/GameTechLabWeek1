#pragma once

#include "pch.h"

class UResourceManager
{
public:
	static UResourceManager& GetInstance();

	void Initialize(const std::string& key, ID3D11Device* device, ID3D11Buffer* vertexBuffer, UINT numVertices);

	UResourceManager(const UResourceManager&) = delete;
	UResourceManager& operator=(const UResourceManager&) = delete;

	ID3D11ShaderResourceView* GetTexture(const std::string& key);
	ID3D11Buffer* GetVertexBuffer(const std::string& key);
	UINT GetNumVertices(const std::string& key);

private:
	UResourceManager() = default;
	~UResourceManager() = default;

private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_vertexBuffers;
	std::unordered_map<std::string, UINT> m_numVertices;
};

