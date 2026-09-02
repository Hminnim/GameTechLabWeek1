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

	UWall(const std::string& meshkey, const FVector spawnLocation, float width);
	virtual ~UWall();
	virtual void Render(URenderer& Renderer) override;
	virtual void SetTexture(ID3D11ShaderResourceView* srv);
	virtual void Update(float DeltaTime, std::vector<UPrimitive*>& others) override;

	virtual void ApplyGravity(float DeltaTime) override {}
	virtual void SetGNumber(float NewG) override {}
	virtual void SetElastic(float NewElastic) override {}
	virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override {}
	virtual void ApplySelfFreeze() override {}
	virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override {}
	virtual void SetEnableAngularMomentum(bool bEnable) override {}
};