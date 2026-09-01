#pragma once

#include "UPrimitive.h"

class UBall : public UPrimitive
{
public:
    static int TotalNumBalls;

    std::string m_textureKey;

    bool bEnableFreeze = false;
    bool isFreezed = false;
    bool isSelfDestruct = false;

    UBall();
    virtual ~UBall();
    virtual void Render(URenderer& Renderer) override;
    virtual void Update(float DeltaTime) override;
    virtual void ApplyGravity(float DeltaTime) override;
    virtual void SetGNumber(float NewG);
    virtual void SetElastic(float NewElastic) override;
    virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override;
    virtual void ApplySizeScaling(float scale) override;
    virtual void ApplySelfDestruct() override;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override;
    virtual void SetEnableAngularMomentum(bool bEnable) override;
};