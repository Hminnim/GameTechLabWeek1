#pragma once

#include "pch.h"
#include "UPrimitive.h"
#include "CollisionManager.h"
#include "SquareData.h"

class UWall : public UPrimitive
{
public:
	static int TotalNumWalls;

	std::string					m_textureKey;
	ID3D11ShaderResourceView* m_textureView;

	ID3D11Buffer* m_vertexBuffer;
	UINT            m_numVertices;

	float Width;

	EPlayer wallowner;

	UWall(const std::string& meshkey, const FVector spawnLocation, float width, EPlayer Owner,float angle);
	virtual ~UWall();
	virtual void Render(URenderer& Renderer) override;
	virtual void SetTexture(ID3D11ShaderResourceView* srv);
	virtual void Update(float DeltaTime, std::vector<UPrimitive*>& others) override;
	virtual void SetElastic(float NewElastic) override {}
	virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce, float MaxDist) override {}
	virtual void ApplySelfFreeze() override {}
	virtual void SetEnableAngularMomentum(bool bEnable) override {}
};