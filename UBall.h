#pragma once

#include "UPrimitive.h"

class UBall : public UPrimitive
{
public:
    static int TotalNumBalls;

    std::string m_textureKey;
	ID3D11ShaderResourceView* m_textureView;

    bool bEnableFreeze = false;
    bool isFreezed = false;
    bool isSelfDestruct = false;
    bool isMagnetActivated = true;

    UBall();
    virtual ~UBall();
    virtual void Render(URenderer& Renderer) override;
    virtual void Update(float DeltaTime, float ScreendWidth, float ScreenHeight) override;
    virtual void ApplyGravity(float DeltaTime) override;
    virtual void SetGNumber(float NewG);
    virtual void SetElastic(float NewElastic) override;
    virtual void SetTexture(ID3D11ShaderResourceView* srv);
    virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override;
    virtual void ApplySizeScaling(float scale) override;
    virtual void ApplyMassScaling(float scale) override;
    virtual void ApplySelfDestruct() override;
    virtual void ApplyEnableFreeze() override;
    virtual void ApplySelfFreeze() override;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override;
    virtual void SetEnableAngularMomentum(bool bEnable) override;
};