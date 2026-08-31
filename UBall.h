#pragma once

#include "UPrimitive.h"

class UBall : public UPrimitive
{
public:
    FVector Location;
    FVector Velocity;
    FVector Rotation;
    FVector AngularVelocity;
    float Radius;
    float Mass;
    static int TotalNumBalls;

    float Elastic;
    float GNumber;

    float Inertia;

    // States
    bool bEnableAngularVelocity;

    UBall();
    virtual ~UBall();
    virtual void Render(URenderer& Renderer) override;
    virtual void Update(float DeltaTime) override;
    virtual void ResolveCollision(UPrimitive* OtherPrimitive) override;
    virtual void ApplyGravity(float DeltaTime) override;
    virtual void SetGNumber(float NewG);
    virtual void SetElastic(float NewElastic) override;
    virtual void ApplyMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) override;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) override;
    virtual void SetEnableAngularMomentum(bool bEnable) override;
};