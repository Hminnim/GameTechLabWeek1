#pragma once
#include "URenderer.h"

class UPrimitive
{
public:
    FVector Location;
    FVector Velocity;
    FVector Rotation;
    FVector AngularVelocity;

    float Radius;
    float Mass;

    float Elastic;
    float GNumber;

    float Inertia;

    // States
    bool bEnableAngularVelocity;

public:
    virtual ~UPrimitive() {}
    virtual void Render(URenderer& Renderer) = 0;
    virtual void Update(float DeltaTime) = 0;
    virtual void ApplyGravity(float DeltaTime) = 0;
    virtual void SetGNumber(float NewG) = 0;
    virtual void SetElastic(float NewElastic) = 0;
    virtual void ApplyReverseMagnetism(UPrimitive* OtherPrimitive, float DeltaTime, float MagneticForce) = 0;
    virtual void ApplyAirResistance(float DeltaTime, float AirResistance) = 0;
    virtual void SetEnableAngularMomentum(bool bEnable) = 0;
};